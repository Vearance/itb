#include <stdio.h>
#include <stdlib.h>
#include "core/database.h"
#include "core/command.h"
#include "core/init.h"
#include "core/tui.h"

int main() {
    tui_print_banner();


    Database *db = (Database *)malloc(sizeof(Database));
    if (db == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for database\n");
        return 1;
    }
    createDatabase(db);
    
    
    if (!initProgram(db)) {
        free(db);
        return 1;
    }
    printf("\n");

    printf("Type 'help;' for available commands or 'exit;' to quit.\n");
    printf("Note: All commands must end with semicolon (;)\n");
    tui_print_prompt();


    char line[1000];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        int line_len = 0;
        while (line[line_len] != '\0' && line[line_len] != '\n') {
            line_len++;
        }
        line[line_len] = 0;
        
        if (line_len > 0) {
            int len = line_len;
            if (len > 0 && line[len-1] != ';') {
                printf("Error: Command must end with semicolon (;)\n");
                tui_print_prompt();
                continue;
            }
            
           
            FILE *tmp = tmpfile();
            if (tmp != NULL) {
                fprintf(tmp, "%s", line);
                rewind(tmp);
                
                STARTKATAFILE(tmp);
                handleCommand(db);
                
               
                while (!EOP) {
                    ADV();
                }
            } else {
                printf("Error: Failed to process command\n");
            }
        }
        
        tui_print_prompt();
    }
    
    free(db);
    return 0;
}