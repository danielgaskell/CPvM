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

// Convenience functions (also saves a few bytes by reducing stack pushes for each use)
void chrout(unsigned char ch) { Shell_CharOutput(termpid, 0, ch); }
void strout(char* str) { Shell_StringOutput(termpid, 0, bnk_vm, str, strlen(str)); }

// Close all open files
void close_all_files(void) {
    char i;
    for (i = 0; i < 8; ++i) {
        if (handles_used[i])
            File_Close(i);
    }
}

// Clean exit, closing files, TPA, and shell as needed
void symbos_exit(void) {
    // close any remaining files
    close_all_files();

    // close TPA
    if (!tpa_error)
        tpa_free();

    // close app/shell
	if (termpid != 0) {
        strout("\r\n"); // since some apps expect this
        Shell_Exit(termpid, 0);
	}
	Sys_Program_End(GET_APPLICATION_ID());
	while (1) Multitasking_SoftInterrupt();
}

// Returns a pointer to the filename (after last backslash) within the full path str
char* tail_after_backslash(char* str) {
    char* result = str + strlen(str);
    while (result > str) {
        if (*result == '\\')
            return result + 1;
        --result;
    }
    return result;
}

// Converts ch to uppercase and returns result
unsigned char toupper(unsigned char ch) {
    if (ch >= 'a' && ch <= 'z')
        return ch - 32;
    else
        return ch;
}

// Converts a filename in the format FILE.EX\x00 (or FILE\x00) to 11-byte padded format (FILE    EX ).
// High bits are not set, so there is no concept of hidden or read-only files.
void filename_to_cpm(char* dest, char* src) {
    unsigned char length_left = 8;

    // set background
    memset(dest, ' ', 11);

    // main filename
    while (length_left && *src && (*src != '.')) {
        *dest++ = toupper(*src++);
        --length_left;
    }
    dest += length_left;

    // extension
    if (*src) ++src; // skip '.' if present
    if (*src) *dest++ = toupper(*src++);
    if (*src) *dest++ = toupper(*src++);
    if (*src) *dest   = toupper(*src);
}

// Converts a filename in 11-byte padded format (FILE    EX ) to FILE.EX\x00 format.
// High bits in the extension will be zeroed.
void filename_to_symbos(char* dest, char* src) {
    unsigned char i;

    // main filename
    i = 8;
    while (i--) {
        if (*src != ' ')
            *dest++ = *src;
        ++src;
    }

    // extension
    *dest++ = '.';
    if ((*src & 0x7F) != ' ') *dest++ = (*src++) & 0x7F; else *dest++ = 0;
    if ((*src & 0x7F) != ' ') *dest++ = (*src++) & 0x7F; else *dest++ = 0;
    if ((*src & 0x7F) != ' ') *dest++ = (*src) & 0x7F;
    *dest = 0;
}

// Parses command tail (either from SymShell or CCP), storing FCB + tail data in large_buffer[].
// If ccp_mode = 0, also initializes termpid and leaves full .COM path in dir_buffer[].
void parse_command_tail(char* command, unsigned char ccp_mode) {
    unsigned char i;
    unsigned char in_quotes, file_on, tail_chars, fcb_start;
    char* tail;
    char file_start[3];

	in_quotes = 0;
	file_on = ccp_mode;
	tail_chars = 0;
	tail = large_buffer + 37;
	memset(large_buffer, 0, 164);
	memset(large_buffer + 1, ' ', 11);
	memset(large_buffer + 17, ' ', 11);
    memset(file_start, 0, 3);
	for (i = 0; command[i]; i++) {
        if (file_on > 1) {
            if (tail_chars < 127) {
                *tail++ = toupper(command[i]);
                ++tail_chars;
            }
        }
	    if (command[i] == 34)
            in_quotes = !in_quotes;
	    if (command[i] == 32 && !in_quotes) {
            command[i] = 0;
            if (!ccp_mode && command[i+1] == 37 && command[i+2] == 115 && command[i+3] == 112) {
                termpid    = (command[i+4 ]-48)*10 + (command[i+5 ]-48);
                termwidth  = (command[i+6 ]-48)*10 + (command[i+7 ]-48);
                termheight = (command[i+8 ]-48)*10 + (command[i+9 ]-48);
                termver    = (command[i+10]-48)*10 + (command[i+11]-48);
                break;
            } else {
                if (file_on < 3)
                    file_start[file_on++] = i + 1;
            }
	    }
	}
    large_buffer[36] = tail_chars;

	// handle errors
	if (termpid == 0)
        symbos_exit();

    // set up FCBs/boot options
	if (file_on) {
        // get .COM file
        if (!ccp_mode)
            Shell_PathAdd(termpid, bnk_vm, 0, command + file_start[0], dir_buffer);

        // copy filenames to FCBs (whether or not they actually exist or are accessible is the program's problem).
        for (i = 1; i < file_on; ++i) {
            fcb_start = (i - 1) * 16;
            if (ccp_mode) {
                if (command[file_start[i]] >= 'A' && command[file_start[i]] <= 'P' && command[file_start[i] + 1] == ':') {
                    // drive specified, use it
                    large_buffer[fcb_start] = command[file_start[i]] - 'A' + 1;
                    filename_to_cpm(large_buffer + fcb_start + 1, command + file_start[i] + 2);
                } else {
                    // drive not specified, use default
                    large_buffer[fcb_start] = 0;
                    filename_to_cpm(large_buffer + fcb_start + 1, command + file_start[i]);
                }
            } else {
                Shell_PathAdd(termpid, bnk_vm, 0, command + file_start[1], buffer);
                if (buffer[0] == dir_buffer[0]) // on same drive as .COM, treat as in default drive
                    large_buffer[fcb_start] = 0;
                else // on different drive, specify drive explicitly
                    large_buffer[fcb_start] = buffer[0] - 64;
                filename_to_cpm(large_buffer + fcb_start + 1, tail_after_backslash(buffer));
            }
        }
    } else {
        // no file specified, prepare for CCP [can only happen on boot]
        launch_ccp = 1;
	}
}

// Select disk e (where 0 = A) as new default
void select_disk(unsigned char e) {
    default_drive = e;
    login_vector |= (1 << e);
    Banking_WriteByte(bnk_tpa, (char*)0x04, e); // set current-drive byte
}
