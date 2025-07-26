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

void ccp_parm_error(void) {
    strcpy(out_buffer, ccp_parameters);
    strcat(out_buffer, "?\r\n");
    strout(out_buffer);
}

// Converts the CCP-style file specification (A:FILE.DAT) in ptr to a SymbOS-style path in dest.
void ccp_path(char* ptr, char* dest) {
    unsigned char drive;
    if (*ptr >= 'A' && *ptr <= 'P' && *(ptr+1) == ':') {
        drive = *ptr - 'A';
        ptr += 2;
    } else {
        drive = default_drive;
    }
    if (drive < 4) {
        strcpy(dest, drive_paths[drive]);
    } else {
        dest[0] = drive + 'A';
        dest[1] = ':';
        dest[2] = '\\';
        dest[3] = 0;
    }
    strcat(dest, ptr);
}

void ccp_dir(void) {
    unsigned char spacing_ticker, max_spacing;
    if (ccp_parameters)
        ccp_path(ccp_parameters, buffer2);
    else
        ccp_path("*.*", buffer2);
    wait_for_async();
    reg_a = Directory_Input(bnk_vm, buffer2, ATTRIB_VOLUME | ATTRIB_DIRECTORY, bnk_vm, dir_buffer, sizeof(dir_buffer), 0, &dir_buffer_count);
    if (!reg_a && dir_buffer_count) {
        dir_buffer_on = 0;
        dir_buffer_ptr = dir_buffer;
        spacing_ticker = 0;
        max_spacing = termwidth / 17;
        while (dir_buffer_on < dir_buffer_count) {
            if (spacing_ticker == 0) {
                out_buffer[0] = default_drive + 'A';
                out_buffer[1] = ':';
                out_buffer[2] = ' ';
                out_buffer[3] = 0;
            } else {
                strcat(out_buffer, "  |  ");
            }
            dir_buffer_ptr += 9;
            filename_to_cpm(buffer2, dir_buffer_ptr);
            buffer2[12] = 0;
            buffer2[11] = buffer2[10];
            buffer2[10] = buffer2[9];
            buffer2[9] = buffer2[8];
            buffer2[8] = ' ';
            strcat(out_buffer, buffer2);
            ++dir_buffer_on;
            dir_buffer_ptr += (strlen(dir_buffer_ptr) + 1);

            ++spacing_ticker;
            if (spacing_ticker >= max_spacing || dir_buffer_on == dir_buffer_count) {
                strcat(out_buffer, "\r\n");
                strout(out_buffer);
                spacing_ticker = 0;
            }
        }
    } else {
        strout("No file\r\n");
    }
}

void ccp_era(void) {
    ccp_path(ccp_parameters, buffer2);
    wait_for_async();
    reg_a = Directory_DeleteFile(bnk_vm, buffer2);
    if (reg_a)
        strout("No file\r\n");
}

void ccp_type(void) {
    unsigned char handle;
    unsigned short read_bytes;
    ccp_path(ccp_parameters, buffer2);
    wait_for_async();
    handle = File_Open(bnk_vm, buffer2);
    if (handle <= 7) {
        read_bytes = 255;
        while (read_bytes == 255) {
            read_bytes = File_Read(handle, bnk_vm, out_buffer, 255);
            out_buffer[read_bytes] = 0;
            strout(out_buffer);
        }
        File_Close(handle);
        strout("\r\n");
    } else {
        ccp_parm_error();
    }
}

void ccp_user(void) {
    unsigned char i;
    i = atoi(ccp_parameters);
    if (i <= 15 && *ccp_parameters >= '0' && *ccp_parameters <= '9') {
        user_number = i;
        strout("(Users not understood by filesystem.)\r\n");
    } else {
        ccp_parm_error();
    }
}

void ccp_ren(void) {
    unsigned char found_equals = 0;
    gen_ptr = ccp_parameters;
    if (*gen_ptr) {
        while (*gen_ptr++) {
            if (*gen_ptr == '=') {
                found_equals = 1;
                *gen_ptr = 0;
            }
        }
    }
    if (found_equals) {
        ccp_path(ccp_parameters, buffer2);
        ccp_path(gen_ptr, dir_buffer);
        gen_ptr = tail_after_backslash(buffer2);
        wait_for_async();
        reg_a = Directory_RenameFile(bnk_vm, dir_buffer, gen_ptr);
        if (reg_a)
            strout("No file\r\n");
    } else {
        strout("No file\r\n");
    }
}

void ccp_save(void) {
    unsigned char pages, handle;
    unsigned char found_space = 0;
    gen_ptr = ccp_parameters;
    if (*gen_ptr) {
        while (*gen_ptr++) {
            if (*gen_ptr == ' ') {
                found_space = 1;
                *gen_ptr = 0;
            }
        }
        if (found_space) {
            pages = atoi(ccp_parameters);
            ccp_path(gen_ptr, buffer2);
            wait_for_async();
            handle = File_New(bnk_vm, buffer2, 0);
            if (handle <= 7) {
                File_Write(handle, bnk_tpa, (char*)0x100, (unsigned short)pages * 256);
                File_Close(handle);
            } else {
                ccp_parm_error();
            }
        } else {
            ccp_parm_error();
        }
    } else {
        ccp_parm_error();
    }
}

void ccp_cd(void) {
    if (*ccp_parameters) {
        if (default_drive < 4) {
            reg_a = strlen(ccp_parameters);
            if (reg_a < 64) {
                gen_ptr = &drive_paths[default_drive][0];
                strcpy(gen_ptr, ccp_parameters);
                if (*(ccp_parameters + reg_a - 1) != '\\')
                    strcat(gen_ptr, "\\");
            } else {
                strout("Path too long\r\n");
            }
        } else {
            strout("Not a redefinable drive\r\n");
        }
    } else {
        ccp_path("", buffer2);
        strcat(buffer2, "\r\n");
        strout(buffer2);
    }
}

void ccp_execute(void) {
    parse_command_tail(ccp_parameters, 2);
    Banking_Copy(bnk_vm, (unsigned short)buffer2, bnk_tpa, 0x5C, 164); // load command-line arguments (generated by symbos_coninit) into base page
    inverse = 0;
    app_run();
}

void ccp_run(void) {
    ccp_path(buffer, buffer2);
    if (!app_load(0x100)) {
        ccp_execute();
    } else {
        strcat(buffer2, ".COM");
        if (!app_load(0x100)) {
            ccp_execute();
        } else {
            strcat(buffer, "?\r\n");
            strout(buffer);
        }
    }
}

void ccp(void) {
    unsigned char i;
    strout("\r\n"); // ensure there is a line break after app exit
    while (1) {
        wait_for_async();
        out_buffer[0] = default_drive + 'A';
        out_buffer[1] = 0;
        if (user_number)
            __itoa(user_number, out_buffer + 1, 10);
        strcat(out_buffer, ">");
        strout(out_buffer);
        strin(buffer);
        cursor_on = 0; // because Shell_StringInput resets it

        // convert command
        ccp_parameters = 0;
        gen_ptr = buffer;
        i = 0;
        while (*gen_ptr) {
            if (*gen_ptr == ' ' && !i) {
                *gen_ptr = 0; // convert first space after command to 0
                ccp_parameters = gen_ptr + 1;
                i = 1;
            }
            *gen_ptr++ = toupper(*gen_ptr);
        }

        // parse command
        if (!strcmp(buffer, "DIR")) {
            ccp_dir();
        } else if (!strcmp(buffer, "ERA")) {
            ccp_era();
        } else if (!strcmp(buffer, "TYPE")) {
            ccp_type();
        } else if (!strcmp(buffer, "USER")) {
            ccp_user();
        } else if (!strcmp(buffer, "REN")) {
            ccp_ren();
        } else if (!strcmp(buffer, "SAVE")) {
            ccp_save();
        } else if (!strcmp(buffer, "CD")) {
            ccp_cd();
        } else if (!strcmp(buffer, "CLS")) {
            // FIXME not currently working as documented in the SymShell manual.
            strout("\x1C\x50\x18\x0C");
            termwidth = 80;
            termheight = 24;
        } else if (!strcmp(buffer, "EXIT")) {
            symbos_exit();
        } else if (buffer[1] == ':' && buffer[2] == 0) {
            select_disk(buffer[0] - 'A');
        } else { // try running command
            ccp_run();
        }
    }
}
