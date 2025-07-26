/*=============================================================================
 CP\/M SymbOS CP/M Compatibility Layer
 (C) Copyright 2024, Prevtenet & Prodatron

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the “Software”), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
=============================================================================*/

// save fcb_buffer back to TPA at fcb
void write_fcb(unsigned short fcb, unsigned char len) { Banking_Copy(bnk_vm, (unsigned short)&fcb_buffer, bnk_tpa, fcb, len); }

// load fcb_buffer from TPA at fcb
void read_fcb(unsigned short fcb) { Banking_Copy(bnk_tpa, fcb, bnk_vm, (unsigned short)&fcb_buffer, sizeof(fcb_buffer)); }

// loads the base path for the specified drive into buffer[]
void set_base_path(unsigned char drive) {
    if (drive == 0 || drive == '?')
        drive = default_drive;
    else
        --drive;
    if (drive < 5) {
        strcpy(buffer, &drive_paths[drive][0]);
    } else {
        buffer[0] = drive + 'A';
        buffer[1] = ':';
        buffer[2] = '\\';
        buffer[3] = 0;
    }
    login_vector |= (1 << (drive - 1));
}

// Converts the filename and path from fcb to SymbOS format, storing in buffer[].
void path_from_fcb(unsigned short fcb) {
    read_fcb(fcb);
    set_base_path(fcb_buffer.drive);
    filename_to_symbos(buffer + strlen(buffer), (char*)&fcb_buffer + 1);
}

// errors if the current path in buffer[] (e.g., after path_from_fcb()) refers to a read-only disk.
unsigned char write_err(void) {
    char* msg;
    if (drive_ro & (1 << (buffer[0] - 65))) {
        msg = "BDOS Err on X: R/O\r\n";
        msg[12] = buffer[0]; // set to correct drive letter
        strout(msg);
        return 1;
    } else {
        return 0;
    }
}

// BDOS #0 - System Reset
void system_reset(void) {
    if (launch_ccp) {
        // pop leftover calls since CCP->app->reset leaves 4 of them on the stack
        __asm
            pop de
            pop de
            pop de
            pop de
        __endasm;
        close_all_files();
        ccp();
    } else {
        symbos_exit();
    }
}

// FIXME: phony dir entry here has problems:
// -- system for finding extent, subsequent extents on wildcards - see RunCPM
// -- fake allocation blocks
unsigned char search_for_next(void) {
    unsigned short file_len;

    if (dir_buffer_on >= dir_buffer_count) {
        // end of files
        return 0xFF;
    } else {
        // construct mock directory entry from current
        file_len = ((*((unsigned long*)dir_buffer_ptr)) + 127) >> 7; // +127 to ensure that last record isn't rounded away
        dir_buffer_ptr += 9;
        memset(&dir_entry, 0, sizeof(dir_entry)); // blank buffer; note that this sets user to 0 (single-user only)
        dir_entry.user = user_number;
        filename_to_cpm(dir_entry.filename, dir_buffer_ptr);
        dir_entry.extent = 0;
        dir_entry.record_count = (file_len > 128) ? 128 : file_len; // FIXME more complex system

        // skip to next entry
        while (*dir_buffer_ptr++);
        ++dir_buffer_on;

        // copy mock directory entry to DMA (lazy approach: always copy to first position in DMA)
        Banking_Copy(bnk_vm, (unsigned short)&dir_entry, bnk_tpa, dma, 32);
        return 0; // 0 = first DMA block
    }
}

unsigned char search_for_first(unsigned short fcb) {
    // extract filename from FCB
    path_from_fcb(fcb);

    // call DIRINP (automatically deals with wildcards)
    dir_buffer_ptr = dir_buffer;
    dir_buffer_on = 0;
    if (!Directory_Input(bnk_vm, buffer, ATTRIB_VOLUME | ATTRIB_DIRECTORY, bnk_vm, dir_buffer, sizeof(dir_buffer), 0, &dir_buffer_count))
        return search_for_next();
    else
        return 0xFF;
}

// Seek file handle to byte position pos if necessary, setting the appropriate extent and current record in fcb_buffer.
// (Does not by itself write changes to fcb_buffer back to TPA.)
void seek_sequential(unsigned char handle, unsigned short pos) {
    unsigned short remaining_records;
    if (handles_pos[handle] != pos) {
        handles_pos[handle] = pos;
        File_Seek(handle, (unsigned long)pos * 128, 0);
        fcb_buffer.current_record = pos & 0x7F;
        fcb_buffer.extent = (pos >> 7) & 0x1F;
        if (fcb_buffer.module & 0x80)
            fcb_buffer.module = (pos >> 12) | 0x80;
        else
            fcb_buffer.module = (pos >> 12);
        remaining_records = handles_len[handle] - handles_pos[handle];
        if (remaining_records < 128)
            fcb_buffer.record_count = remaining_records;
        else
            fcb_buffer.record_count = 128;
    }
}

// returns the current byte position within the file referenced in fcb_buffer
unsigned short current_pos(void) {
    return ((unsigned short)(fcb_buffer.module & 0x7F) * 4096) + ((unsigned short)fcb_buffer.extent * 128) + fcb_buffer.current_record;
}

// closes and marks as reclaimed the FCB pointed to by handle (assumed to be open/valid)
void close_and_reclaim(unsigned char handle) {
    unsigned char i;
    i = Banking_ReadByte(bnk_tpa, ((char*)handles_fcb[handle]) + 13) - 1;
    if (i == handle) // sanity check: only mark this as reclaimed if it still points to the handle (probably not overwritten)
        Banking_WriteByte(bnk_tpa, ((char*)handles_fcb[handle] + 13), 0xFF);
    File_Close(handle);
    handles_used[handle] = 0;
    handles_fcb[handle] = 0;
}

// closes/marks as reclaimed any other open files matching the FCB-style filename at addr (for subsequent delete/rename)
void purge_file(char* addr) {
    unsigned char i;
    for (i = 0; i < 8; ++i) {
        if (handles_used[i]) {
            Banking_Copy(bnk_tpa, handles_fcb[i], bnk_vm, (unsigned short)buffer2, 12);
            if (!memcmp(addr, buffer2, 12))
                close_and_reclaim(i);
        }
    }
}

// finds a file handle that can be marked as reclaimed; reclaims and returns 1 on success
unsigned char reclaim_handle(void) {
    unsigned char i, t;

    // pass 1: look for an FCB whose handle no longer matches what is on file
    for (i = 0; i < 8; ++i) {
        if (handles_used[i]) {
            t = Banking_ReadByte(bnk_tpa, (char*)handles_fcb[i] + 13) - 1;
            if (t != i) {
                close_and_reclaim(i);
                return 1;
            }
        }
    }

    // pass 2: look for any read-only FCB
    for (i = 0; i < 8; ++i) {
        if (handles_used[i]) {
            t = Banking_ReadByte(bnk_tpa, (char*)handles_fcb[i] + 14);
            if (t & 0x80) {
                close_and_reclaim(i);
                return 1;
            }
        }
    }

    // found nothing reclaimable, fail
    return 0;
}

// checks to see if the FCB in fcb_buffer has had its handle reclaimed, and if so, re-opens/re-syncs it.
// also fires if the FCB handle/address does not match what is on file (*stares pointedly at Hi-Tech C*)
void reopen_reclaimed(unsigned short fcb) {
    unsigned char handle;
    handle = fcb_buffer.handle - 1;
    if (handle == 0xFE || !handles_used[handle] || handles_fcb[handle] != fcb) {
        path_from_fcb(fcb);
        reopen_reclaimed_try:
        handle = File_Open(bnk_vm, buffer);
        if (handle <= 7) {
            // store new handle and seek file to existing FCB location
            fcb_buffer.handle = handle + 1;
            Banking_WriteByte(bnk_tpa, (char*)fcb + 13, handle + 1);
            handles_used[handle] = 1;
            handles_fcb[handle] = fcb;
            handles_pos[handle] = 0;
            handles_len[handle] = (File_Seek(handle, 0, 2) + 127) >> 7; // get file length (in records); +127 ensure that incomplete records are not rounded away
            File_Seek(handle, 0, 0); // seek back to start
            seek_sequential(handle, current_pos());
        } else {
            if (symbos_info.msgBuffer[7] == 128) { // happens on "too many files open" (note: internal to SDK, does not match documented return message)
                if (reclaim_handle())
                    goto reopen_reclaimed_try;
            }
        }
    }
}

// close file at FCB, if any (matching either by handle or known FCB address); returns 0 on success, 0xFF on failure
unsigned char close_fcb(unsigned short fcb) {
    unsigned char handle;
    if (fcb_buffer.handle >= 1 && fcb_buffer.handle <= 8) {
        handle = fcb_buffer.handle - 1;
        if (handles_used[handle]) {
            close_and_reclaim(handle);
            return 0;
        }
    }
    for (handle = 0; handle < 8; ++handle) {
        if (handles_fcb[handle] == fcb && handles_used[handle]) {
            close_and_reclaim(handle);
            return 0;
        }
    }
    return 0xFF;
}

// update a newly-opened FCB with file/handle information and save it (also updates external handle information)
void new_fcb(unsigned short fcb, unsigned char handle) {
    handles_used[handle] = 1;
    handles_pos[handle] = 0;
    handles_fcb[handle] = fcb;
    fcb_buffer.handle = handle + 1;
    seek_sequential(handle, current_pos()); // deal with files opened with extent + record already at a specific location
    fcb_buffer.record_count = handles_len[handle] > 128 ? 128 : handles_len[handle];
    memset(&fcb_buffer.dirinfo, 0, 16);
    write_fcb(fcb, 33);
}

// FIXME: advanced programs may try to match file extent as well as name, to open partway through; handle this better.
// FIXME simulate allocation blocks?
unsigned char open_file(unsigned short fcb) {
    unsigned char handle;

    // extract filename from FCB
    path_from_fcb(fcb);

    // close prior usage of this FCB
    close_fcb(fcb);

    // if it has a wildcard, use DIRINP to get first filename
    if (strchr(buffer, '?')) {
        if (!Directory_Input(bnk_vm, buffer, ATTRIB_VOLUME | ATTRIB_DIRECTORY, bnk_vm, buffer2, 23, 0, &dir_buffer_count_open)) {
            if (dir_buffer_count_open) {
                // found at least one matching file, create absolute path
                set_base_path(fcb_buffer.drive);
                strcat(buffer, buffer2 + 9);
            } else {
                return 0xFF;
            }
        } else {
            return 0xFF;
        }
    }

    // try opening file
    open_file_try:
    handle = File_Open(bnk_vm, buffer);
    if (handle <= 7) {
        handles_len[handle] = (File_Seek(handle, 0, 2) + 127) >> 7; // get file length (in records); +127 ensure that incomplete records are not rounded away
        File_Seek(handle, 0, 0); // seek back to start
        fcb_buffer.module = 0x80; // unmodified, module 0 (this prevents seeking directly to a module, but not all apps are good about zeroing this byte before load!)
        new_fcb(fcb, handle);
        return 0;
    } else {
        if (symbos_info.msgBuffer[7] == 128) { // happens on "too many files open" (note: internal to SDK, does not match documented return message)
            if (reclaim_handle())
                goto open_file_try;
        }
        return 0xFF; // FIXME more specific message
    }
}

unsigned char new_file(unsigned short fcb) {
    unsigned char f;

    // extract filename from FCB
    path_from_fcb(fcb);

    // close prior usage of this FCB
    close_fcb(fcb);

    // try creating file
    if (!write_err()) {
        new_file_try:
        f = File_New(bnk_vm, buffer, 0);
        if (f <= 7) {
            handles_len[f] = 0;
            handles_pos[f] = 0;
            fcb_buffer.extent = 0;
            fcb_buffer.current_record = 0;
            fcb_buffer.module = 0; // starts already not "unmodified"
            fcb_buffer.random_record = 0;
            fcb_buffer.random_record_overflow = 0;
            new_fcb(fcb, f);
            return 0;
        } else {
            if (reclaim_handle())
                goto new_file_try;
            else // error (FIXME more specific message)
                return 0xFF;
        }
    } else {
        return 0xFF;
    }
}

// FIXME: check for File_Close error codes
unsigned char close_file(unsigned short fcb) {
    read_fcb(fcb);
    return close_fcb(fcb);
}

unsigned char delete_file(unsigned short fcb) {
    path_from_fcb(fcb);
    purge_file((char*)&fcb_buffer);
    close_fcb(fcb);
    if (!write_err()) {
        if (!Directory_DeleteFile(bnk_vm, buffer))
            return 0;
        else
            return 0xFF;
    } else {
        return 0xFF;
    }
}

unsigned char rename_file(unsigned short pseudo_fcb) {
    path_from_fcb(pseudo_fcb);
    purge_file((char*)&fcb_buffer);
    purge_file(((char*)&fcb_buffer) + 16);
    close_fcb(pseudo_fcb);
    filename_to_symbos(buffer2, ((char*)&fcb_buffer) + 17);
    if (!write_err()) {
        if (!Directory_RenameFile(bnk_vm, buffer, buffer2))
            return 0;
        else
            return 0xFF;
    } else {
        return 0xFF;
    }
}

void advance_record(unsigned short fcb) {
    ++fcb_buffer.current_record;
    if (fcb_buffer.current_record > 127) {
        fcb_buffer.current_record = 0; // Note that RunCPM increments after 128 and sets current_record to 1 instead, contrary to Johnson-Laird.
        ++fcb_buffer.extent;
    }
    if (fcb_buffer.extent > 31) {
        fcb_buffer.extent = 0;
        ++fcb_buffer.module;
    }
    write_fcb(fcb, 33);
}

// where read_bytes < 128, fill remaining part of record with 0x1A (EOF) - improves compatibility with FAT files that end mid-record
void fill_eof(unsigned char read_bytes) {
    memset(buffer, 0x1A, 128 - read_bytes);
    Banking_Copy(bnk_vm, (unsigned short)buffer, bnk_tpa, dma + read_bytes, 128 - read_bytes);
}

unsigned char read_sequential(unsigned short fcb) {
    unsigned char handle, read_bytes;
    read_fcb(fcb);
    reopen_reclaimed(fcb);
    handle = fcb_buffer.handle - 1;

    if (handle <= 7) {
        seek_sequential(handle, current_pos()); // ensure all positional information is updated to match FCB
        if (handles_pos[handle] < handles_len[handle]) {
            // read to DMA
            read_bytes = File_Read(handle, bnk_tpa, (unsigned char*)dma, 128);
            if (read_bytes == 0)
                return 1; // EOF (nothing was read, error or EOF)
            if (read_bytes < 128)
                fill_eof(read_bytes);

            // advance current record
            ++handles_pos[handle];
            advance_record(fcb);

            // return success
            return 0; // OK
        } else {
            return 1; // EOF
        }
    } else {
        return 9; // invalid FCB
    }
}

unsigned char write_sequential(unsigned short fcb) {
    unsigned char handle, wrote_bytes;
    unsigned short pos;
    read_fcb(fcb);
    reopen_reclaimed(fcb);
    handle = fcb_buffer.handle - 1;

    if (handle <= 7) {
        if (!write_err()) {
            pos = current_pos();
            seek_sequential(handle, pos); // ensure all positional information is updated to match FCB
            wrote_bytes = File_Write(handle, bnk_tpa, (unsigned char*)dma, 128);
            if (wrote_bytes < 128)
                return 1; // directory full (something went wrong, unable to finish write)

            // advance current record
            ++handles_len[handle];
            ++handles_pos[handle];
            fcb_buffer.module &= 0x7F; // clear unmodified flag
            ++fcb_buffer.record_count;
            advance_record(fcb);

            // return success
            return 0;
        } else {
            return 0xFF; // hardware error
        }
    } else {
        return 9; // invalid FCB
    }
}

void update_record_to_random(unsigned short fcb) {
    fcb_buffer.current_record = (fcb_buffer.random_record & 0x7F);
    fcb_buffer.extent = ((fcb_buffer.random_record >> 7) & 0x1F);
    if (fcb_buffer.module & 0x80)
        fcb_buffer.module = (fcb_buffer.random_record >> 12) | 0x80;
    else
        fcb_buffer.module = fcb_buffer.random_record >> 12;
    write_fcb(fcb, 36);
}

unsigned char read_random(unsigned short fcb) {
    unsigned char handle, read_bytes;
    read_fcb(fcb);
    reopen_reclaimed(fcb);
    handle = fcb_buffer.handle - 1;

    if (handle <= 7) {
        update_record_to_random(fcb);
        if (fcb_buffer.random_record < handles_len[handle]) {
            if (handles_pos[handle] != fcb_buffer.random_record) {
                handles_pos[handle] = fcb_buffer.random_record;
                File_Seek(handle, (unsigned long)fcb_buffer.random_record * 128, 0);
            }
            read_bytes = File_Read(handle, bnk_tpa, (unsigned char*)dma, 128);
            ++handles_pos[handle];
            if (read_bytes == 0)
                return 1; // nothing was read (returned as "attempted to read unwritten record")
            if (read_bytes < 128)
                fill_eof(read_bytes);
            return 0;
        } else {
            return 1; // attempted to read unwritten record
        }
    } else {
        return 9; // invalid FCB
    }
}

unsigned char write_random(unsigned short fcb) {
    unsigned char handle, wrote_bytes;
    read_fcb(fcb);
    reopen_reclaimed(fcb);
    handle = fcb_buffer.handle - 1;

    if (handle <= 7) {
        if (!write_err()) {
            if (handles_len[handle] < fcb_buffer.random_record) {
                // too short, extend file
                File_Seek(handle, 0, 2); // seek to end
                memset(buffer, 0, 128); // create null bytes to write
                while (handles_len[handle] < fcb_buffer.random_record) {
                    File_Write(handle, bnk_vm, buffer, 128);
                    ++handles_len[handle];
                }
                handles_pos[handle] = fcb_buffer.random_record; // already at end, so no seek needed after this; just update handles_pos
            }
            if (handles_pos[handle] != fcb_buffer.random_record) {
                handles_pos[handle] = fcb_buffer.random_record;
                File_Seek(handle, (unsigned long)fcb_buffer.random_record * 128, 0);
            }
            wrote_bytes = File_Write(handle, bnk_tpa, (unsigned char*)dma, 128);
            if (wrote_bytes < 128)
                return 2;// directory full (unable to finish write)
            ++handles_len[handle];
            ++handles_pos[handle];
            fcb_buffer.module &= 0x7F; // clear unmodified flag
            update_record_to_random(fcb);
            return 0;
        } else {
            return 0xFF; // write-protected (hardware error)
        }
    } else {
        return 9; // invalid FCB
    }
}

unsigned char get_file_size(unsigned short fcb) {
    unsigned char handle;
    read_fcb(fcb);
    handle = fcb_buffer.handle - 1;

    if (handle <= 7 && handles_used[handle]) {
        // file is already open, get size from existing record
        fcb_buffer.random_record = handles_len[handle];
    } else {
        // file is not already open, get size from directory info
        path_from_fcb(fcb);
        if (!Directory_Input(bnk_vm, buffer, ATTRIB_VOLUME | ATTRIB_DIRECTORY, bnk_vm, buffer2, 23, 0, &dir_buffer_count_open)) {
            fcb_buffer.random_record = ((*(unsigned long*)buffer2) + 127) >> 7;
        } else {
            return 0xFF;
        }
    }
    if (fcb_buffer.random_record == 65535) // FIXME: technically this should be 65536 - how to track that?
        fcb_buffer.random_record_overflow = 1;
    else
        fcb_buffer.random_record_overflow = 0;
    write_fcb(fcb, 36);
    return 0;
}

void set_random_record(unsigned short fcb) {
    unsigned char handle;
    read_fcb(fcb);
    reopen_reclaimed(fcb);
    handle = fcb_buffer.handle - 1;
    fcb_buffer.random_record = handles_pos[handle];
    if (handles_pos[handle] == 65535)
        fcb_buffer.random_record_overflow = 1;
    else
        fcb_buffer.random_record_overflow = 0;
    write_fcb(fcb, 36);
}

// terminal emulation for a single character - handles regular printable chars
// and maintains a state machine for escape sequences, which will be outputted
// all at once on completion of the sequence. Output goes at *out_ptr.
void buffer_char(unsigned char ch) {
    reg_a = ch & 0x7F; // strip high bit, since some software will send with high bit set
    if (reg_a == 27) {
        // escape code start
        escape_chars_expected = 1;
        escape_char_bracket = 0;
        escape_char_position = 0;
        escape_attrib_mode = 0;
    } else if (escape_chars_expected) {
        // escape code continuation
        if (escape_attrib_mode) {
            adm_attribs[adm_foreground] = ((reg_a - '0') & 4); // "reverse" bit of ADM-3A attribute
        } else if (escape_char_position == 2) {
            escape_char_y = reg_a;
            escape_char_position = 1;
        } else if (escape_char_position == 1) {
            if (reg_a == 0x20 && escape_char_y == 0x20) { // behavior of 1F 20 20 is inconsistent (?), so send equivalent 1E instead
                *out_ptr++ = 0x1E;
            } else {
                *out_ptr++ = 0x1F;
                *out_ptr++ = reg_a - 31;
                *out_ptr++ = escape_char_y - 31;
            }
            escape_chars_expected = 0;
        } else if (reg_a == '[') {
            escape_char_bracket = 1;
            ansi_parms[1] = 0;
            ansi_parms[4] = 0;
        } else if (reg_a == '=' || reg_a == 'Y') {
            escape_char_position = 2;
        } else if (reg_a == '(') {
            inverse = adm_attribs[0];
            adm_foreground = 1;
            escape_chars_expected = 0;
        } else if (reg_a == ')') {
            inverse = adm_attribs[1];
            adm_foreground = 0;
            escape_chars_expected = 0;
        } else if (reg_a == 'G') {
            escape_attrib_mode = 1;
            escape_chars_expected = 1;
        } else if (reg_a == 'p') {
            inverse = 1;
            escape_chars_expected = 0;
        } else if (reg_a == 'q') {
            inverse = 0;
            escape_chars_expected = 0;
        } else if (escape_char_bracket) {
            if (reg_a == ';') {
                ansi_parms[escape_char_bracket] = 0;
                escape_char_bracket = 4;
            } else if (reg_a >= '0' && reg_a <= '9') {
                ansi_parms[escape_char_bracket++] = reg_a;
            } else {
                ansi_parms[escape_char_bracket] = 0;
                ansi_parm1 = atoi(&ansi_parms[1]);
                ansi_parm2 = atoi(&ansi_parms[4]);
                if (reg_a == 'A') {
                    *out_ptr = 0x0E;
                    *out_ptr++ = ansi_parm1 + 185;
                } else if (reg_a == 'B') {
                    *out_ptr = 0x0E;
                    *out_ptr++ = ansi_parm1 + 160;
                } else if (reg_a == 'C') {
                    *out_ptr = 0x0E;
                    *out_ptr++ = ansi_parm1;
                } else if (reg_a == 'D') {
                    *out_ptr = 0x0E;
                    *out_ptr++ = ansi_parm1 + 80;
                } else if (reg_a == 'H') {
                    *out_ptr++ = 0x1F;
                    *out_ptr++ = ansi_parm2 ? ansi_parm2 : 1;
                    *out_ptr = ansi_parm1 ? ansi_parm1 : 1;
                } else if (reg_a == 'K') {
                    if (ansi_parms[1] == '2') {
                        *out_ptr++ = 0x11;
                        *out_ptr = 0x12;
                    } else if (ansi_parms[1] == '1') {
                        *out_ptr = 0x11;
                    } else {
                        *out_ptr = 0x12;
                    }
                } else if (reg_a == 'J') {
                    *out_ptr = 0x0C; // only "clear full screen" version currently supported
                } else if (reg_a == 'f') {
                    *out_ptr = 0x1E;
                } else if (reg_a == 'm') {
                    inverse = ansi_parm1;
                }
                out_ptr++;
                escape_chars_expected = 0;
            }
        } else {
            gen_ptr = *(unsigned short**)(((unsigned short*)&escapes) + reg_a - '(');
            strcpy(out_ptr, gen_ptr);
            out_ptr += strlen(gen_ptr);
            escape_chars_expected = 0;
        }
    } else if (reg_a < 32) {
        // single-character control code, translate
        if (reg_a == 31) {
            // Soroc IQ-120 "new line" (used by, e.g., Turbo Pascal)
            *out_ptr++ = 0x0A;
            *out_ptr++ = 0x0D;
        } else {
            *out_ptr++ = control_codes[reg_a];
        }
    } else {
        // regular character, emit as-is
        if (inverse && reg_a == ' ')
            *out_ptr++ = 0xB1;
        else
            *out_ptr++ = reg_a;
    }
}

void output_buffer(void) {
    if (out_buffer[0]) {
        *out_ptr = 0; // zero-terminate
        strout(out_buffer);
    }
}

void refresh_out_buffer(void) {
    out_ptr = out_buffer;
    *out_ptr = 0;
    if (cursor_on) {
        *out_ptr++ = 2; // turn cursor off
        cursor_on = 0;
    }
}

// Prints the string in buffer addr to console, converting VT-52 escape sequences / ADM-3A codes to SymShell format.
// Note: CP/M will convert character 9 inline to spaces, even when used as a control code; this can probably be ignored,
// but it means that programs may send the binary number 9 (e.g., for addresses) with the high bit set (stripped above).
void print_to_terminal(char* addr) {
    refresh_out_buffer();
    while (*addr)
        buffer_char(*addr++);
    output_buffer();
}

void console_output(unsigned char ch) {
    refresh_out_buffer();
    buffer_char(ch);
    output_buffer();
}

void string_output(unsigned short addr) {
    unsigned char scanning = 1;
    buffer[192] = 0; // ensure string is zero-terminated

    // output string at addr in 255-byte chunks
    while (scanning) {
        // get a chunk
        Banking_Copy(bnk_tpa, addr, bnk_vm, (unsigned short)&buffer[0], 192);

        // terminate at $, if found
        gen_ptr = buffer;
        while (*gen_ptr) {
            if (*gen_ptr == '$') {
                scanning = 0;
                *gen_ptr = 0;
                break;
            }
            ++gen_ptr;
        }

        // print what we found
        print_to_terminal(buffer);
        addr += 192;
    }
}

// get a character from the console, remapping control chars as specified in keyconv[]
unsigned char convert_key(unsigned char ch) {
    if (ch >= 127 && ch <= 139)
        ch = keyconv[ch - 127];
    ch &= 0x7F;
    return ch;
}

unsigned char bios_console_input(void) {
    if (!cursor_on) {
        chrout(3); // cursor on
        cursor_on = 1;
    }
    reg_a = convert_key(chrin());
    return reg_a;
}

unsigned char console_input(void) {
    if (!cursor_on) {
        chrout(3); // cursor on
        cursor_on = 1;
    }
    reg_a = convert_key(chrin());
    if (reg_a == 13)
        reg_a = 10; // CP/M programs expect ENTER to be 10 for this input type only
    chrout(reg_a);
    return reg_a;
}

unsigned char direct_io(unsigned char e) {
    reg_a = e;
    if (e == 0xFF) {
        if (!cursor_on) {
            chrout(3); // cursor on
            cursor_on = 1;
        }
        reg_a = convert_key(Shell_CharTest(termpid, 0, 1));
    } else {
        buffer[0] = e;
        buffer[1] = 0;
        print_to_terminal(buffer);
    }
    return reg_a;
}

unsigned char read_string(unsigned short addr) {
    unsigned char max_chars = Banking_ReadByte(bnk_tpa, (char*)addr);
    reg_a = strin(buffer + 1);
    cursor_on = 0; // because Shell_StringInput resets it
    if (reg_a) { // Ctrl+C
        __asm
            pop de
        __endasm; // pop call from this routine
        system_reset();
    }
    buffer[0] = strlen(buffer + 1); // save read length to NC byte
    Banking_Copy(bnk_vm, (unsigned short)&buffer[0], bnk_tpa, addr + 1, max_chars + 1); // copy to TPA, including NC byte
    return buffer[0];
}

unsigned char console_status(void) {
    if (Shell_CharTest(termpid, 0, 0))
        return 0xFF;
    else
        return 0x00;
}

void flush_buffer(unsigned short de) {
    Banking_Copy(bnk_tpa, de, bnk_vm, (unsigned short)buffer, 129);
    print_to_terminal(buffer);
}

unsigned char reset_disk_system(void) {
    dma = 0x0080;
    login_vector = 0;
    drive_ro = 0;
    default_drive = 0;
    return 0;
}

// Main BDOS entry point: translates inter-bank calling convention (L, DE) to __sdcccall(1) convention (A, DE).
// This is a lazy way of getting named register variables in bdos_calls(); putting this as its own routine
// (which falls through to bdos_calls() because it is immediately before it and __naked) keeps the
// relevant code from being optimized out of existence.
void BDOS(void) __naked {
    __asm
        ld a, l
        di              ;restore VM-side shadow registers
    __endasm;
    __asm__ ("ex af,af'"); // because the __asm parser doesn't like apostrophes (derp)
    __asm
vmshda::ld a,#0
    __endasm;
    __asm__ ("ex af,af'");
    __asm
		exx
vmshdb::ld bc,#0
vmshdd::ld de,#0
vmshdh::ld hl,#0
		exx
		ei
    __endasm;
} // FALL THROUGH

// Main switchyard for BDOS calls
void bdos_calls(unsigned char c, unsigned short de) __naked {
    #ifdef TRACE
    if (c != 255) {
        out_buffer[0] = '[';
        out_buffer[1] = 0;
        __itoa(c, out_buffer + 1, 10);
        strcat(out_buffer, "]");
        strout(out_buffer);
    }
    #endif // TRACE

    reg_hl = 0; // default return value
    if (c == 255) {
        __asm
            rst #0x38             ;special function 255 -> interrupt occured, call scheduler
            jp finish_bdos
        __endasm;
    } else if (c == 254) {
        system_reset();
    } else if (c == 253) {
        flush_buffer(de);
    } else if (c == 252) {
        reg_hl = bios_console_input(); // unlike normal BDOS #1, does not echo result
    } else {
        wait_for_async(); // theoretically only required before shell calls, but generically waiting here avoids some hard-to-track-down freezes
        switch (c) {
        case 0: // System Reset
            system_reset();
            break;
        case 1: // Console Input
            reg_hl = console_input();
            break;
        case 2: // Console Output
            console_output(de);
            break;
        case 3: // Reader Input
            reg_hl = 0x1F;
            break;
        case 4: // Punch Output
            // do nothing
            break;
        case 5: // List Output
            // do nothing
            break;
        case 6: // Direct Console I/O
            reg_hl = direct_io(de);
            break;
        case 7: // Get I/O Byte
            reg_hl = Banking_ReadByte(bnk_tpa, (unsigned char*)0x03);
            break;
        case 8: // Set I/O Byte
            Banking_WriteByte(bnk_tpa, (unsigned char*)0x03, de);
            break;
        case 9: // Print String
            string_output(de);
            break;
        case 10: // Read Console Buffer
            reg_hl = read_string(de);
            break;
        case 11: // Get Console Status
            reg_hl = console_status();
            break;
        case 12: // Return Version Number
            reg_hl = 0x0022;
            break;
        case 13: // Reset Disk System
            reg_hl = reset_disk_system();
            break;
        case 14: // Select Disk
            reg_hl = select_disk(de);
            break;
        case 15: // Open File
            reg_hl = open_file(de);
            break;
        case 16: // Close File
            reg_hl = close_file(de);
            break;
        case 17: // Search for First
            reg_hl = search_for_first(de);
            break;
        case 18: // Search for Next
            reg_hl = search_for_next();
            break;
        case 19: // Delete File
            reg_hl = delete_file(de);
            break;
        case 20: // Read Sequential
            reg_hl = read_sequential(de);
            break;
        case 21: // Write Sequential
            reg_hl = write_sequential(de);
            break;
        case 22: // Make File
            reg_hl = new_file(de);
            break;
        case 23: // Rename File
            reg_hl = rename_file(de);
            break;
        case 24: // Return Log-in Vector
            reg_hl = login_vector;
            break;
        case 25: // Return Current Disk
            reg_hl = default_drive;
            break;
        case 26: // Set DMA Address
            dma = de;
            break;
        case 27: // Get Allocation Vector
            // "Digital Research considers the actual layout of the allocation vector to be proprietary information." - Johnson-Laird
            reg_hl = 0x80; // Not actually faked right now (just pointed to a real address) since only DR system applications should care.
            break;
        case 28: // Write Protect Disk
            drive_ro |= (1 << default_drive);
            break;
        case 29: // Get Read-Only Vector
            reg_hl = drive_ro;
            break;
        case 30: // Set File Attributes
            // do nothing
            break;
        case 31: // Get Disk Parameter Block
            reg_hl = DPBADDR;
            break;
        case 32: // Set/Get User Code
            if ((unsigned char)de == 0xFF)
                reg_hl = user_number;
            else
                user_number = de;
            break;
        case 33: // Read Random
            reg_hl = read_random(de);
            break;
        case 34: // Write Random
            reg_hl = write_random(de);
            break;
        case 35: // Compute File Size
            reg_hl = get_file_size(de);
            break;
        case 36: // Set Random Record
            set_random_record(de);
            break;
        case 37: // Reset Drive
            drive_ro &= (~de);
            login_vector &= (~de); // Note that RunCPM does not do this, although my reading of Johnson-Laird is that it should.
            break;
        case 40: // Write Random w/Fill
            reg_hl = write_random(de); // (no actual disk blocks, so just behaves like Write Random)
            break;
        #ifdef UNIMPLEMENTED
        default:
            refresh_out_buffer();
            strcpy(out_ptr, "[UNIMPLEMENTED: ");
            out_ptr += strlen(out_ptr);
            __itoa(c, out_ptr, 10);
            strcat(out_ptr, "]");
            strout(out_buffer);
            break;
        #endif
        }
    }

    #ifdef TRACE
    out_buffer[0] = '=';
    __itoa(reg_hl, out_buffer + 1, 16);
    strout(out_buffer);
    wait_for_async();
    #endif // TRACE

    // load return values into HL and return (will be copied to BA TPA-side)
    __asm
    finish_bdos::
        ld hl, (_reg_hl)
		di              ;save VM-side shadow registers
    __endasm;
    __asm__ ("ex af,af'"); // because the __asm parser doesn't like apostrophes (derp)
    __asm
        ld (vmshda+1),a
    __endasm;
    __asm__ ("ex af,af'");
    __asm
		exx
		ld (vmshdb+1),bc
		ld (vmshdd+1),de
		ld (vmshdh+1),hl
		exx
        jp jmp_bnkret
    __endasm;
}
