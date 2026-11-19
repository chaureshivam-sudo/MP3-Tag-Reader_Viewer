#include <stdio.h>

#include "types.h"
#include "view.h"
#include "edit.h"
#include "colours.h"

int main(int argc, char* argv[])
{
    TagInfo mp3tagInfo;
    TagData mp3tagData;

    if (argc<2){
        printf (RED "ERROR: Incorrect format of Command Line Arguments.\n" RESET);
        printf (CYAN "INFO: Use .\\a.exe/a.out -help or -h for Help menu.\n" RESET);
    }
    else{
        OperationType ret = check_operation (argv);	//To check which Operation needs to be performed based on the Option passed in Command Line.

        if (ret==p_view){
            printf (BLUE "--------------------------\nMP3 Tag Reader and Editor\n--------------------------\n" RESET);
            // validate all the arguments passed in Command Line
            if (read_and_validate_mp3_file (argv, &mp3tagInfo) == p_success){	//If validation is successful, proceed with the Viewing process.
                //To execute the Viewing process of the MP3 Tag.
                if (view_tag (argv, &mp3tagInfo) == p_success){
                    printf (GREEN "INFO: Done.\n" RESET);
                    printf (BLUE "--------------------------\n" RESET);
                }
            }
        }
        else if(ret==p_edit){
            printf (BLUE "MP3 Tag Reader and Editor:\n" RESET);
            printf (BLUE "--------------------------\n" RESET);
            //To validate all the arguments passed in Command Line.
            if (read_and_validate_mp3_file_args(argv,&mp3tagData) == p_success){	//If validation is successful, proceed with the Editing process.
                //To execute the Editing process of the MP3 Tag.
                if (edit_tag(argv,&mp3tagData) == p_success)
                {
                    printf (GREEN "INFO: Done.\n" RESET);
                    printf (BLUE "--------------------------\n" RESET);
                    //Call the view_tag function to display the Updated content.
                }
            }
        }
        else if(ret==p_help){
            printf (BLUE "INFO: Help Menu for Tag Reader and Editor:\n" RESET);
            printf (CYAN "For Viewing the Tags -> .\\a.exe/a.out -v <file_name.mp3>\n" RESET);
            printf (CYAN "For Editing the Tags -> .\\a.exe/a.out -e <modifier> \"New_Value\" <file_name.mp3>\n" RESET);
            printf (YELLOW "INFO: Modifier Functions:\n" RESET);
            printf (YELLOW "-t" RESET "\t Title Tag\n"
                    YELLOW "-a" RESET "\t Artist Tag\n"
                    YELLOW "-A" RESET "\t Album Tag\n"
                    YELLOW "-y" RESET "\t Year Tag\n"
                    YELLOW "-d" RESET "\t Content Type Tag\n"
                    YELLOW "-c" RESET "\t Comments Tag\n");
        }
        else if(ret==p_unsupported){
            printf (RED "ERROR: Unsupported Operation.\n" RESET);
            printf (CYAN "INFO: Use .\\a.exe/a.out -help or -h for Help menu.\n" RESET);
        }
    }
    return 0;
}
