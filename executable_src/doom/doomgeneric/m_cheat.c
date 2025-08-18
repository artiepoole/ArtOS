// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>


#include <string.h>

#include "doomtype.h"
#include "m_cheat.h"

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
int
cht_CheckCheat
(cheatseq_t* cht,
 char key)
{
    // if we make a short sequence on a cheat with parameters, this 
    // will not work in vanilla doom.  behave the same.

    if (cht->parameter_chars > 0 && strlen(cht->sequence) < cht->sequence_len)
        return false;

    if (cht->chars_read < strlen(cht->sequence))
    {
        // still reading characters from the cheat code
        // and verifying.  reset back to the beginning 
        // if a key is wrong

        if (key == cht->sequence[cht->chars_read])
            ++cht->chars_read;
        else
            cht->chars_read = 0;

        cht->param_chars_read = 0;
    }
    else if (cht->param_chars_read < cht->parameter_chars)
    {
        // we have passed the end of the cheat sequence and are 
        // entering parameters now 

        cht->parameter_buf[cht->param_chars_read] = key;

        ++cht->param_chars_read;
    }

    if (cht->chars_read >= strlen(cht->sequence)
        && cht->param_chars_read >= cht->parameter_chars)
    {
        cht->chars_read = cht->param_chars_read = 0;

        return true;
    }

    // cheat not matched yet

    return false;
}

void
cht_GetParam
(cheatseq_t* cht,
 char* buffer)
{
    memcpy(buffer, cht->parameter_buf, cht->parameter_chars);
}
