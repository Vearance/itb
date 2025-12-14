#ifndef TUI_H
#define TUI_H

void tui_print_banner();
void tui_print_prompt();
void tui_print_section(const char *title);
void tui_print_frame(const char *title, int width);
void tui_print_box(const char *content, int width);
void tui_print_divider(int width, char ch);
void tui_clear_screen();
void tui_loading(const char *message);
void tui_print_info_box(const char *title, const char *content);
void tui_print_success(const char *message);
void tui_print_error(const char *message);
void tui_print_warning(const char *message);
void tui_print_table_header(const char *headers[], int col_count, int widths[]);
void tui_print_table_row(const char *cells[], int col_count, int widths[]);
void tui_print_table_footer(int widths[], int col_count);
void tui_print_list_item(int index, const char *content, int width);
void tui_print_comment_frame(int indent, const char *author, const char *content, int upvotes, int downvotes, int width);
void tui_print_nav_hint(const char *hint);
int tui_interactive_menu(const char *title, const char *options[], int option_count);
void tui_print_breadcrumb(const char *items[], int count);
void tui_print_pagination(int current_page, int total_pages, int items_per_page, int total_items);
void tui_print_status_bar(const char *left_info, const char *right_info);

#endif


