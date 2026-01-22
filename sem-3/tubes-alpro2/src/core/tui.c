#include "tui.h"
#include <stdio.h>

#define TUI_RESET "\033[0m"
#define TUI_PRIMARY "\033[1;36m"
#define TUI_SECONDARY "\033[1;35m"
#define TUI_ACCENT "\033[1;33m"
#define TUI_SUCCESS "\033[1;32m"
#define TUI_ERROR "\033[1;31m"
#define TUI_WARNING "\033[1;33m"
#define TUI_INFO "\033[1;34m"
#define TUI_BORDER "\033[0;36m"

static int my_strlen(const char *s) {
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

static void print_repeat(char ch, int count) {
    for (int i = 0; i < count; i++) {
        putchar(ch);
    }
}

void tui_print_banner() {
    printf(TUI_PRIMARY "================================================================\n");
    printf(" Welcome to " TUI_ACCENT "Groddit" TUI_PRIMARY "! Dive into anything (not illegal ofc).\n");
    printf("================================================================\n" TUI_RESET);
    printf(TUI_SECONDARY "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⠀⠤⠤⠤⠀⣀⠀⠀⠀⠀⠀⠀⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡠⠐⢉⠴⣚⠉⣙⠢⢄⡤⢞⡂⢀⣐⢄⠀⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡔⡤⣞⢁⠊⢀⣀⠐⢿⡄⠰⢁⡀⠈⠺⣦⢡⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⣗⣟⡸⠀⠻⡿⠃⢸⣇⢃⠿⠿⠀⠀⣽⢸⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠈⠙⢷⣴⡀⠀⠠⣪⣾⣷⡄⡀⠠⣐⢕⠁⠀\n");
    printf("⠀⢰⡦⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠙⠲⡖⠓⠉⠁⠈⠉⠒⠀⠈⢸⠀⠀\n");
    printf("⢶⣿⣷⣤⢀⣀⡀⠀⠀⣏⡑⠢⢄⠀⠀⠀⠈⠐⠀⠐⠀⠀⠀⠀⠀⡸⡀⠀\n");
    printf("⠛⠛⠛⠟⠀⠤⠤⠌⢉⠀⠈⠓⢬⣿⣦⡤⣤⣤⠤⠤⣤⣤⣤⣤⣚⣔⣄⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⡤⠂⠀⠀⢀⠤⠤⢄⡨⠔⠒⢍⠉⢁⣯⡆\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⡗⢤⡤⣬⠀⠀⠀⢇⠀⠀⠀⠁⠀⠀⡸⢰⣿⣿⡿\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⢌⡿⣽⡀⠀⠀⠈⠒⢄⡀⠀⢀⠔⠁⠈⠙⡋⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠑⠳⢧⣠⣤⣄⣠⣀⣈⣱⡥⠤⠴⠦⠴⠃⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⣿⣿⠀⣿⣿⣿⣄⠀⠀⠀⠀⠀⠀\n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠉⠉⠀⠈⠉⠉⠉⠀⠀⠀⠀⠀⠀\n" TUI_RESET);
    printf("\n");
}

void tui_print_prompt() {
    printf(TUI_ACCENT "> " TUI_RESET);
    fflush(stdout);
}

void tui_print_section(const char *title) {
    printf("\n" TUI_PRIMARY "=== %s ===" TUI_RESET "\n", title);
}

void tui_print_frame(const char *title, int width) {
    if (width < 10) width = 70;
    int title_len = my_strlen(title);
    int padding = (width - title_len - 2) / 2;
    
    printf(TUI_BORDER "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
    
    printf(TUI_BORDER "|" TUI_RESET);
    print_repeat(' ', padding);
    printf(TUI_PRIMARY "%s" TUI_RESET, title);
    print_repeat(' ', width - 2 - padding - title_len);
    printf(TUI_BORDER "|\n" TUI_RESET);
    
    printf(TUI_BORDER "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
}

void tui_print_box(const char *content, int width) {
    if (width < 10) width = 70;
    int content_len = my_strlen(content);
    
    printf(TUI_BORDER "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
    
    printf(TUI_BORDER "|" TUI_RESET);
    int spaces = width - 2 - content_len;
    int left_spaces = spaces / 2;
    print_repeat(' ', left_spaces);
    printf("%s", content);
    print_repeat(' ', spaces - left_spaces);
    printf(TUI_BORDER "|\n" TUI_RESET);
    
    printf(TUI_BORDER "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
}

void tui_print_divider(int width, char ch) {
    if (width < 10) width = 70;
    printf(TUI_BORDER);
    print_repeat(ch, width);
    printf(TUI_RESET "\n");
}

void tui_clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void tui_loading(const char *message) {
    const char *spinner = "|/-\\";
    static int spin_idx = 0;
    printf("\r" TUI_INFO "%s %c" TUI_RESET, message, spinner[spin_idx % 4]);
    fflush(stdout);
    spin_idx++;
    // Note: Sleep removed to comply with allowed libraries
}

void tui_print_info_box(const char *title, const char *content) {
    int width = 70;
    int title_len = my_strlen(title);
    int content_len = my_strlen(content);
    int max_len = (title_len > content_len) ? title_len : content_len;
    if (max_len + 4 > width) width = max_len + 4;
    
    printf(TUI_INFO "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
    
    printf(TUI_INFO "| " TUI_PRIMARY "%s" TUI_RESET, title);
    print_repeat(' ', width - 3 - title_len);
    printf(TUI_INFO "|\n" TUI_RESET);
    
    printf(TUI_INFO "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
    
    printf(TUI_INFO "| " TUI_RESET "%s", content);
    print_repeat(' ', width - 3 - content_len);
    printf(TUI_INFO "|\n" TUI_RESET);
    
    printf(TUI_INFO "+");
    print_repeat('-', width - 2);
    printf("+\n" TUI_RESET);
}

void tui_print_success(const char *message) {
    printf(TUI_SUCCESS "✓ %s" TUI_RESET "\n", message);
}

void tui_print_error(const char *message) {
    printf(TUI_ERROR "✗ %s" TUI_RESET "\n", message);
}

void tui_print_warning(const char *message) {
    printf(TUI_WARNING "⚠ %s" TUI_RESET "\n", message);
}

void tui_print_table_header(const char *headers[], int col_count, int widths[]) {
    printf(TUI_BORDER "┌");
    for (int i = 0; i < col_count; i++) {
        print_repeat('-', widths[i]);
        if (i < col_count - 1) printf("┬");
    }
    printf("┐\n" TUI_RESET);
    
    printf(TUI_BORDER "│" TUI_RESET);
    for (int i = 0; i < col_count; i++) {
        printf(TUI_PRIMARY " %-*s " TUI_RESET, widths[i] - 2, headers[i]);
        printf(TUI_BORDER "│" TUI_RESET);
    }
    printf("\n");
    
    printf(TUI_BORDER "├");
    for (int i = 0; i < col_count; i++) {
        print_repeat('-', widths[i]);
        if (i < col_count - 1) printf("┼");
    }
    printf("┤\n" TUI_RESET);
}

void tui_print_table_row(const char *cells[], int col_count, int widths[]) {
    printf(TUI_BORDER "│" TUI_RESET);
    for (int i = 0; i < col_count; i++) {
        printf(" %-*s ", widths[i] - 2, cells[i] ? cells[i] : "");
        printf(TUI_BORDER "│" TUI_RESET);
    }
    printf("\n");
}

void tui_print_table_footer(int widths[], int col_count) {
    printf(TUI_BORDER "└");
    for (int i = 0; i < col_count; i++) {
        print_repeat('-', widths[i]);
        if (i < col_count - 1) printf("┴");
    }
    printf("┘\n" TUI_RESET);
}

void tui_print_list_item(int index, const char *content, int width) {
    if (width < 10) width = 70;
    char index_str[10];
    snprintf(index_str, sizeof(index_str), "%d.", index);
    
    printf(TUI_BORDER "│" TUI_RESET " " TUI_ACCENT "%-3s" TUI_RESET " %s", index_str, content);
    int content_len = my_strlen(content);
    int total_len = 3 + 1 + content_len;
    int padding = width - 2 - total_len;
    if (padding > 0) {
        print_repeat(' ', padding);
    }
    printf(TUI_BORDER "│\n" TUI_RESET);
}

void tui_print_comment_frame(int indent, const char *author, const char *content, int upvotes, int downvotes, int width) {
    if (width < 10) width = 70;
    int indent_width = indent * 2;
    int content_width = width - indent_width - 4;
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    printf(TUI_BORDER "┌");
    print_repeat('-', content_width);
    printf("┐\n" TUI_RESET);
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf(TUI_BORDER "│" TUI_RESET " " TUI_ACCENT "%s" TUI_RESET, author);
    int author_len = my_strlen(author);
    int padding = content_width - 2 - author_len - 15;
    if (padding > 0) {
        print_repeat(' ', padding);
    }
    printf(TUI_SUCCESS "↑%d" TUI_RESET " " TUI_ERROR "↓%d" TUI_RESET " ", upvotes, downvotes);
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf(TUI_BORDER "│" TUI_RESET " %s", content);
    int content_len = my_strlen(content);
    padding = content_width - 2 - content_len;
    if (padding > 0) {
        print_repeat(' ', padding);
    }
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf(TUI_BORDER "└");
    print_repeat('-', content_width);
    printf("┘\n" TUI_RESET);
}

void tui_print_nav_hint(const char *hint) {
    printf("\n" TUI_INFO " " TUI_RESET "%s\n", hint);
}

int tui_interactive_menu(const char *title, const char *options[], int option_count) {
    if (option_count == 0) return -1;
    
    tui_print_frame(title, 70);
    
    for (int i = 0; i < option_count; i++) {
        printf(TUI_BORDER "│" TUI_RESET "  " TUI_ACCENT "%d." TUI_RESET " %s", i + 1, options[i]);
        int padding = 70 - 6 - my_strlen(options[i]);
        if (padding > 0) {
            print_repeat(' ', padding);
        }
        printf(TUI_BORDER "│\n" TUI_RESET);
    }
    
    printf(TUI_BORDER "└");
    for (int i = 0; i < 68; i++) printf("─");
    printf("┘\n" TUI_RESET);
    
    printf("\n" TUI_INFO "Pilih opsi (1-%d): " TUI_RESET, option_count);
    fflush(stdout);
    
    int choice;
    if (scanf("%d", &choice) == 1) {
        if (choice >= 1 && choice <= option_count) {
            return choice - 1;
        }
    }
    return -1;
}

void tui_print_breadcrumb(const char *items[], int count) {
    if (count == 0) return;
    
    printf(TUI_BORDER " " TUI_RESET);
    for (int i = 0; i < count; i++) {
        if (i > 0) {
            printf(TUI_BORDER " > " TUI_RESET);
        }
        if (i == count - 1) {
            printf(TUI_PRIMARY "%s" TUI_RESET, items[i]);
        } else {
            printf("%s", items[i]);
        }
    }
    printf("\n");
}

void tui_print_pagination(int current_page, int total_pages, int items_per_page, int total_items) {
    if (total_pages <= 1) return;
    
    int start_item = (current_page - 1) * items_per_page + 1;
    int end_item = current_page * items_per_page;
    if (end_item > total_items) end_item = total_items;
    
    printf("\n" TUI_BORDER "┌");
    for (int i = 0; i < 68; i++) printf("─");
    printf("┐\n" TUI_RESET);
    
    printf(TUI_BORDER "│" TUI_RESET " Halaman: " TUI_ACCENT "%d" TUI_RESET " / %d", current_page, total_pages);
    int padding = 70 - 20 - 10;
    printf(" | Item: %d-%d dari %d", start_item, end_item, total_items);
    padding = 70 - 2 - 50;
    if (padding > 0) {
        print_repeat(' ', padding);
    }
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    printf(TUI_BORDER "│" TUI_RESET " ");
    if (current_page > 1) {
        printf(TUI_ACCENT "[< Prev]" TUI_RESET);
    } else {
        printf("      ");
    }
    printf(" ");
    if (current_page < total_pages) {
        printf(TUI_ACCENT "[Next >]" TUI_RESET);
    } else {
        printf("       ");
    }
    padding = 70 - 2 - 20;
    if (padding > 0) {
        print_repeat(' ', padding);
    }
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    printf(TUI_BORDER "└");
    for (int i = 0; i < 68; i++) printf("─");
    printf("┘\n" TUI_RESET);
}

void tui_print_status_bar(const char *left_info, const char *right_info) {
    printf(TUI_BORDER "┌");
    for (int i = 0; i < 68; i++) printf("─");
    printf("┐\n" TUI_RESET);
    
    printf(TUI_BORDER "│" TUI_RESET " %s", left_info ? left_info : "");
    int left_len = left_info ? my_strlen(left_info) : 0;
    int right_len = right_info ? my_strlen(right_info) : 0;
    int padding = 70 - 3 - left_len - right_len;
    if (padding > 0) {
        print_repeat(' ', padding);
    }
    if (right_info) {
        printf("%s", right_info);
    }
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    printf(TUI_BORDER "└");
    for (int i = 0; i < 68; i++) printf("─");
    printf("┘\n" TUI_RESET);
}


