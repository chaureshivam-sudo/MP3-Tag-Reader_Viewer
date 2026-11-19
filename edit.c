#include <stdio.h>
#include <string.h>
#include "types.h"
#include "edit.h"
#include "colours.h"

/*
 Simple idea of this file:
 An mp3 file has:  [10 byte header] [frames: title, artist, album...] [song]
 To change one frame we do 2 things:
   1) copy everything to temp.mp3, but put the new value in that one frame
   2) copy temp.mp3 back to song.mp3
*/


//   Flip the 4 bytes of a number.
//   The mp3 file saves numbers in reverse order, so we flip to read/write them.
//   Example: 00 00 00 0B  <->  0B 00 00 00 
static void swap_bytes (uint* num)
{
    char* ptr = (char*) num;
    for (int i=0;i<2;i++)
    {
        char temp = *(ptr+i);
        *(ptr+i) = *(ptr+4-i-1);
        *(ptr+4-i-1) = temp;
    }
}


/* Show the list of options (-t, -a, -A, -y, -d, -c) */
static void print_modifiers (void)
{
    printf (YELLOW "INFO: Modifier Functions:\n" RESET);
    printf (YELLOW "-t" RESET "\t Title Tag\n"
            YELLOW "-a" RESET "\t Artist Tag\n"
            YELLOW "-A" RESET "\t Album Tag\n"
            YELLOW "-y" RESET "\t Year Tag\n"
            YELLOW "-d" RESET "\t Content Type Tag\n"
            YELLOW "-c" RESET "\t Comments Tag\n");
}


/* Check what the user typed:  ./a.out -e -t "New value" song.mp3
                                       [1] [2]   [3]       [4]   */
Status read_and_validate_mp3_file_args(char* argv[],TagData* mp3tagData)
{

    // 1. check the option (-t, -a, -A, -y, -d, -c) 
    if (argv[2]==NULL)
    {
        printf (CYAN "INFO: For Editing the Tags -> .\\a.out -e <modifier> \"New_Value\" <file_name.mp3>\n" RESET);
        print_modifiers ();
        return p_failure;
    }

    // save the frame name for that option 
    if      (strncmp(argv[2],"-t",2)==0) strncpy(mp3tagData->frame_Id,"TIT2",5);   // title
    else if (strncmp(argv[2],"-a",2)==0) strncpy(mp3tagData->frame_Id,"TPE1",5);   // artist
    else if (strncmp(argv[2],"-A",2)==0) strncpy(mp3tagData->frame_Id,"TALB",5);   // album
    else if (strncmp(argv[2],"-y",2)==0) strncpy(mp3tagData->frame_Id,"TYER",5);   // year
    else if (strncmp(argv[2],"-d",2)==0) strncpy(mp3tagData->frame_Id,"TCON",5);   // content type
    else if (strncmp(argv[2],"-c",2)==0) strncpy(mp3tagData->frame_Id,"COMM",5);   // comment
    else
    {
        printf (RED "ERROR: Unsupported Modifier.\n" RESET);
        print_modifiers ();
        return p_failure;
    }

    /* 2. check the new value */
    if (argv[3]==NULL)
    {
        printf (RED "ERROR: New_Value to be updated on the Frame ID %s is Empty.\n" RESET, mp3tagData->frame_Id);
        printf (CYAN "INFO: For Editing the Tags -> .\\a.out -e <modifier> \"New_Value\" <file_name.mp3>\n" RESET);
        return p_failure;
    }

    /* size = length of the text + 1 (1 extra byte is stored before the text) */
    mp3tagData->frame_Id_size = strlen(argv[3])+1;

    if (mp3tagData->frame_Id_size >= 100){          // too long for our array
        printf (RED "ERROR: Length of the Data is too Long!.\n" RESET);
        return p_failure;
    }
    strncpy (mp3tagData->frame_Id_value, argv[3], mp3tagData->frame_Id_size);   // save new text

    /* 3. check the mp3 file */
    if (argv[4]==NULL)            //file name missing
    {
        printf (CYAN "INFO: For Editing the Tags -> .\\a.out -e <modifier> \"New_Value\" <file_name.mp3>\n" RESET);
        return p_failure;
    }
    //open the MP3 file
    mp3tagData->fptr_mp3 = fopen(argv[4],"r");
    if (mp3tagData->fptr_mp3 == NULL){              // file not found
        printf (RED "ERROR: Unable to Open the %s file.\n" RESET, argv[4]);
        return p_failure;
    }

    char str[3];
    fread (str,1,3,mp3tagData->fptr_mp3);           // read first 3 letters
    fclose (mp3tagData->fptr_mp3);

    if (strncmp (str,"ID3",3) != 0){                // a tagged mp3 starts with "ID3"
        printf (RED "ERROR: The file Signature is not matching with that of a '.mp3' file.\n" RESET);
        return p_failure;
    }

    printf (YELLOW "Frame ID to change :" RESET " %s\n", mp3tagData->frame_Id);
    printf (YELLOW "New value          :" RESET " %s\n", mp3tagData->frame_Id_value);
    return p_success;
}


/* Do the editing */
Status edit_tag (char* argv[], TagData* mp3tagData)
{

    mp3tagData->fptr_mp3  = fopen (argv[4], "r");       // song.mp3  -> we only read
    mp3tagData->fptr_temp = fopen ("temp.mp3", "w");    // temp.mp3  -> we only write

    /* copy the 10 byte header */
    char str[10];
    fread  (str,1,10,mp3tagData->fptr_mp3);
    fwrite (str,1,10,mp3tagData->fptr_temp);

    /* go through the 6 frames one by one */
    char* frame_tag [6] = {"TIT2", "TPE1", "TALB", "TYER", "TCON", "COMM"};

    for (int i=0;i<6;i++)
    {
        /* copy the frame name (4 letters) */
        fread  (str,1,4,mp3tagData->fptr_mp3);
        fwrite (str,1,4,mp3tagData->fptr_temp);

        /* flag = 1 -> this is the frame to change
           flag = 0 -> not our frame, just copy it */
        char flag = (strncmp(mp3tagData->frame_Id, frame_tag[i], 4) == 0);

        if (copy_data (mp3tagData, flag) == p_failure)
        {
            printf (RED "ERROR: Unable to copy the Frame Data from Source to temp '.mp3' file.\n" RESET);
            return p_failure;
        }
    }

    /* copy the rest of the file (the song) */
    if (copy_remaining_data (mp3tagData) == p_failure)
    {
        printf (RED "ERROR: Unable to copy the remaining Data from Source to temp '.mp3' file.\n" RESET);
        return p_failure;
    }

    fclose (mp3tagData->fptr_mp3);
    fclose (mp3tagData->fptr_temp);

    /* open again, now temp.mp3 is read and song.mp3 is written */
    mp3tagData->fptr_mp3  = fopen (argv[4], "w");
    mp3tagData->fptr_temp = fopen ("temp.mp3", "r");

    /* copy temp.mp3 back to song.mp3 */
    if (copy_back_to_src (mp3tagData) == p_failure)
    {
        printf (RED "ERROR: Unable to copy the Data from temp.mp3 to Source '.mp3' file.\n" RESET);
        return p_failure;
    }

    fclose (mp3tagData->fptr_mp3);
    fclose (mp3tagData->fptr_temp);
    return p_success;
}


/* Copy ONE frame (its name is already copied).
   A frame is:  [4 byte size] [3 bytes extra] [text]
   flag = 1 -> write the NEW text
   flag = 0 -> copy the OLD text */
Status copy_data (TagData* mp3tagData, char flag)
{
    uint old_size;
    char f[3];

    /* read the old size and flip it so it becomes a normal number */
    fread (&old_size,4,1,mp3tagData->fptr_mp3);
    swap_bytes (&old_size);

    if (flag==1){
        /* ---- this frame is changed ---- */

        uint new_size = mp3tagData->frame_Id_size;      // keep a normal copy of new size

        /* flip the new size and write it */
        swap_bytes (&mp3tagData->frame_Id_size);
        fwrite (&mp3tagData->frame_Id_size,4,1,mp3tagData->fptr_temp);

        /* copy the 3 extra bytes */
        fread  (f,1,3,mp3tagData->fptr_mp3);
        fwrite (f,1,3,mp3tagData->fptr_temp);

        /* write the new text */
        fwrite (mp3tagData->frame_Id_value,1,(new_size-1),mp3tagData->fptr_temp);

        /* skip the old text in song.mp3 */
        fseek  (mp3tagData->fptr_mp3,(long)(old_size-1),SEEK_CUR);
    }
    else
    {
        /* ---- this frame is not changed, just copy ---- */

        /* flip the size back and write it */
        swap_bytes (&old_size);
        fwrite (&old_size,4,1,mp3tagData->fptr_temp);
        swap_bytes (&old_size);                         // make it a normal number again

        /* copy the 3 extra bytes */
        fread  (f,1,3,mp3tagData->fptr_mp3);
        fwrite (f,1,3,mp3tagData->fptr_temp);

        /* copy the old text */
        char str[old_size-1];
        fread  (str,1,(old_size-1),mp3tagData->fptr_mp3);
        fwrite (str,1,(old_size-1),mp3tagData->fptr_temp);
    }
    return p_success;
}


/* Copy everything that is left (the song) from song.mp3 to temp.mp3 */
Status copy_remaining_data (TagData* mp3tagData)
{
    char ch;

    while (fread(&ch,1,1,mp3tagData->fptr_mp3)){        // stops at end of file
        fwrite (&ch,1,1,mp3tagData->fptr_temp);
    }
    return p_success;
}


/* Copy the whole temp.mp3 back to song.mp3 */
Status copy_back_to_src (TagData* mp3tagData)
{
    char ch;

    /* go to the start of both files */
    fseek (mp3tagData->fptr_mp3,0,SEEK_SET);
    fseek (mp3tagData->fptr_temp,0,SEEK_SET);

    while (fread(&ch,1,1,mp3tagData->fptr_temp))
    {
        fwrite (&ch,1,1,mp3tagData->fptr_mp3);
    }
    return p_success;
}