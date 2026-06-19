#define _POSIX_C_SOURCE 200809L

#include "visualizer.h"

#include "utils/magi_error.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIS_NODE_WIDTH 126.0
#define VIS_NODE_HEIGHT 126.0
#define VIS_ICON_SIZE 86.0
#define VIS_LABEL_WIDTH 156.0
#define VIS_LABEL_HEIGHT 38.0
#define VIS_COLUMN_HOST 150.0
#define VIS_COLUMN_SWITCH 460.0
#define VIS_COLUMN_ROUTER 770.0
#define VIS_TOP_MARGIN 168.0
#define VIS_ROW_GAP 172.0
#define VIS_LEFT_MARGIN 48.0
#define VIS_RIGHT_MARGIN 150.0

typedef struct VisualNode {
  const TopologyNodeInfo* info;
  double x;
  double y;
} VisualNode;

typedef struct VisualNodeCollectState {
  VisualNode* items;
  size_t count;
  size_t index;
} VisualNodeCollectState;

typedef struct VisualLinkCollectState {
  TopologyLinkInfo** items;
  size_t count;
  size_t index;
} VisualLinkCollectState;

static bool has_suffix(const char* text, const char* suffix) {
  if (text == NULL || suffix == NULL) {
    return false;
  }

  size_t text_len = strlen(text);
  size_t suffix_len = strlen(suffix);
  if (suffix_len > text_len) {
    return false;
  }

  return strcmp(text + text_len - suffix_len, suffix) == 0;
}

static void collect_visual_node(const char* key, void* value, void* ctx) {
  (void)key;

  VisualNodeCollectState* state = (VisualNodeCollectState*)ctx;
  if (state == NULL || state->index >= state->count || value == NULL) {
    return;
  }

  state->items[state->index].info = (const TopologyNodeInfo*)value;
  state->items[state->index].x = 0.0;
  state->items[state->index].y = 0.0;
  state->index++;
}

static void collect_visual_link(const char* key, void* value, void* ctx) {
  (void)key;

  VisualLinkCollectState* state = (VisualLinkCollectState*)ctx;
  if (state == NULL || state->index >= state->count || value == NULL) {
    return;
  }

  state->items[state->index++] = (TopologyLinkInfo*)value;
}

static int compare_visual_node(const void* lhs, const void* rhs) {
  const VisualNode* left = (const VisualNode*)lhs;
  const VisualNode* right = (const VisualNode*)rhs;

  if (left->info->kind != right->info->kind) {
    return (int)left->info->kind - (int)right->info->kind;
  }

  return strcmp(left->info->node->name, right->info->node->name);
}

static int compare_visual_link(const void* lhs, const void* rhs) {
  const TopologyLinkInfo* left = *(const TopologyLinkInfo* const*)lhs;
  const TopologyLinkInfo* right = *(const TopologyLinkInfo* const*)rhs;

  int comparison = strcmp(left->node_a, right->node_a);
  if (comparison != 0) {
    return comparison;
  }

  if (left->port_a != right->port_a) {
    return (int)left->port_a - (int)right->port_a;
  }

  comparison = strcmp(left->node_b, right->node_b);
  if (comparison != 0) {
    return comparison;
  }

  if (left->port_b != right->port_b) {
    return (int)left->port_b - (int)right->port_b;
  }

  return 0;
}

static VisualNode* collect_nodes(const Topology* topology, size_t* count_out) {
  if (topology == NULL || topology->nodes == NULL || count_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  *count_out = topology->nodes->count;
  size_t alloc_count = *count_out > 0U ? *count_out : 1U;
  VisualNode* nodes = calloc(alloc_count, sizeof(*nodes));
  if (nodes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  VisualNodeCollectState state = {.items = nodes, .count = *count_out, .index = 0U};
  hashmap_foreach(topology->nodes, collect_visual_node, &state);
  *count_out = state.index;
  qsort(nodes, *count_out, sizeof(*nodes), compare_visual_node);
  return nodes;
}

static TopologyLinkInfo** collect_links(const Topology* topology, size_t* count_out) {
  if (topology == NULL || topology->links == NULL || count_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  *count_out = topology->links->count;
  size_t alloc_count = *count_out > 0U ? *count_out : 1U;
  TopologyLinkInfo** links = calloc(alloc_count, sizeof(*links));
  if (links == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  VisualLinkCollectState state = {.items = links, .count = *count_out, .index = 0U};
  hashmap_foreach(topology->links, collect_visual_link, &state);
  *count_out = state.index;
  qsort(links, *count_out, sizeof(*links), compare_visual_link);
  return links;
}

static const VisualNode* find_visual_node(const VisualNode* nodes, size_t count,
                                          const char* name) {
  if (nodes == NULL || name == NULL) {
    return NULL;
  }

  for (size_t index = 0U; index < count; ++index) {
    if (nodes[index].info != NULL && nodes[index].info->node != NULL &&
        strcmp(nodes[index].info->node->name, name) == 0) {
      return &nodes[index];
    }
  }

  return NULL;
}

static double x_for_kind(TopologyNodeKind kind) {
  switch (kind) {
  case TOPOLOGY_NODE_HOST:
    return VIS_COLUMN_HOST;
  case TOPOLOGY_NODE_SWITCH:
    return VIS_COLUMN_SWITCH;
  case TOPOLOGY_NODE_ROUTER:
    return VIS_COLUMN_ROUTER;
  default:
    return VIS_COLUMN_SWITCH;
  }
}

static void assign_node_positions(VisualNode* nodes, size_t count) {
  size_t row_by_kind[3] = {0U, 0U, 0U};

  for (size_t index = 0U; index < count; ++index) {
    TopologyNodeKind kind = nodes[index].info->kind;
    size_t row = 0U;
    if (kind >= TOPOLOGY_NODE_HOST && kind <= TOPOLOGY_NODE_ROUTER) {
      row = row_by_kind[(size_t)kind]++;
    }

    nodes[index].x = x_for_kind(kind);
    nodes[index].y = VIS_TOP_MARGIN + (double)row * VIS_ROW_GAP;
  }
}

static size_t max_kind_count(const VisualNode* nodes, size_t count) {
  size_t counts[3] = {0U, 0U, 0U};

  for (size_t index = 0U; index < count; ++index) {
    TopologyNodeKind kind = nodes[index].info->kind;
    if (kind >= TOPOLOGY_NODE_HOST && kind <= TOPOLOGY_NODE_ROUTER) {
      counts[(size_t)kind]++;
    }
  }

  size_t max_count = counts[0];
  if (counts[1] > max_count) {
    max_count = counts[1];
  }
  if (counts[2] > max_count) {
    max_count = counts[2];
  }

  return max_count > 0U ? max_count : 1U;
}

static void svg_write_escaped(FILE* out, const char* text) {
  if (out == NULL || text == NULL) {
    return;
  }

  for (const char* cursor = text; *cursor != '\0'; ++cursor) {
    switch (*cursor) {
    case '&':
      fputs("&amp;", out);
      break;
    case '<':
      fputs("&lt;", out);
      break;
    case '>':
      fputs("&gt;", out);
      break;
    case '"':
      fputs("&quot;", out);
      break;
    case '\'':
      fputs("&apos;", out);
      break;
    default:
      fputc(*cursor, out);
      break;
    }
  }
}

static void dot_write_escaped(FILE* out, const char* text) {
  if (out == NULL || text == NULL) {
    return;
  }

  for (const char* cursor = text; *cursor != '\0'; ++cursor) {
    if (*cursor == '"' || *cursor == '\\') {
      fputc('\\', out);
    }
    fputc(*cursor, out);
  }
}

static bool uri_is_unreserved(unsigned char ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
         ch == '-' || ch == '_' || ch == '.' || ch == '~';
}

static bool write_svg_data_uri(FILE* out, const char* path) {
  static const char hex[] = "0123456789ABCDEF";

  FILE* icon = fopen(path, "rb");
  if (icon == NULL) {
    return false;
  }

  fputs("data:image/svg+xml;utf8,", out);
  int value = fgetc(icon);
  while (value != EOF) {
    unsigned char ch = (unsigned char)value;
    if (uri_is_unreserved(ch)) {
      fputc((int)ch, out);
    } else {
      fputc('%', out);
      fputc(hex[(ch >> 4U) & 0x0FU], out);
      fputc(hex[ch & 0x0FU], out);
    }
    value = fgetc(icon);
  }

  bool ok = ferror(icon) == 0;
  fclose(icon);
  return ok;
}

static const char* color_for_kind(TopologyNodeKind kind) {
  switch (kind) {
  case TOPOLOGY_NODE_HOST:
    return "#dff4e5";
  case TOPOLOGY_NODE_SWITCH:
    return "#e2efff";
  case TOPOLOGY_NODE_ROUTER:
    return "#fff0cf";
  default:
    return "#eeeeee";
  }
}

static const char* stroke_for_kind(TopologyNodeKind kind) {
  switch (kind) {
  case TOPOLOGY_NODE_HOST:
    return "#328a48";
  case TOPOLOGY_NODE_SWITCH:
    return "#2f65a7";
  case TOPOLOGY_NODE_ROUTER:
    return "#b56b00";
  default:
    return "#666666";
  }
}

static const char* icon_path_for_kind(TopologyNodeKind kind) {
  switch (kind) {
  case TOPOLOGY_NODE_HOST:
    return "doc/host.svg";
  case TOPOLOGY_NODE_SWITCH:
    return "doc/switch.svg";
  case TOPOLOGY_NODE_ROUTER:
    return "doc/router.svg";
  default:
    return "";
  }
}

static void write_svg_icon(FILE* out, TopologyNodeKind kind, double x, double y) {
  const char* icon_path = icon_path_for_kind(kind);
  if (icon_path[0] == '\0') {
    fprintf(out,
            "    <circle cx=\"%.1f\" cy=\"%.1f\" r=\"24\" fill=\"#ffffff\" "
            "stroke=\"%s\" stroke-width=\"2\"/>\n",
            x + VIS_ICON_SIZE / 2.0, y + VIS_ICON_SIZE / 2.0, stroke_for_kind(kind));
    return;
  }

  fprintf(out,
          "    <image href=\"");
  if (!write_svg_data_uri(out, icon_path)) {
    svg_write_escaped(out, icon_path);
  }
  fprintf(out,
          "\" x=\"%.1f\" y=\"%.1f\" width=\"%.1f\" height=\"%.1f\" "
          "preserveAspectRatio=\"xMidYMid meet\"/>\n",
          x, y, VIS_ICON_SIZE, VIS_ICON_SIZE);
}

static void write_svg_node(FILE* out, const VisualNode* visual) {
  const TopologyNodeInfo* info = visual->info;
  double top = visual->y - VIS_NODE_HEIGHT / 2.0;
  double icon_x = visual->x - VIS_ICON_SIZE / 2.0;
  double icon_y = top;

  fputs("  <g class=\"node\">\n", out);
  write_svg_icon(out, info->kind, icon_x, icon_y);

  fprintf(out,
          "    <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
          "font-size=\"17\" font-weight=\"700\" fill=\"#1f2933\">",
          visual->x, icon_y + VIS_ICON_SIZE + 18.0);
  svg_write_escaped(out, info->node->name);
  fputs("</text>\n", out);

  fprintf(out,
          "    <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
          "font-size=\"12\" fill=\"#39434d\">",
          visual->x, icon_y + VIS_ICON_SIZE + 34.0);
  svg_write_escaped(out, topology_kind_name(info->kind));
  fputs("</text>\n", out);

  if (info->kind == TOPOLOGY_NODE_HOST && info->ip_address[0] != '\0') {
    fprintf(out,
            "    <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
            "font-size=\"11\" fill=\"#39434d\">",
            visual->x, icon_y + VIS_ICON_SIZE + 49.0);
    svg_write_escaped(out, info->ip_address);
    fputs("</text>\n", out);
  } else if (info->kind == TOPOLOGY_NODE_SWITCH && info->num_ports > 0U) {
    fprintf(out,
            "    <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
            "font-size=\"11\" fill=\"#39434d\">ports=%u</text>\n",
            visual->x, icon_y + VIS_ICON_SIZE + 49.0, (unsigned)info->num_ports);
  }

  fputs("  </g>\n", out);
}

static double visual_abs(double value) {
  return value < 0.0 ? -value : value;
}

static bool link_should_be_straight(const VisualNode* from, const VisualNode* to) {
  if (from->x == to->x) {
    return visual_abs(from->y - to->y) <= VIS_ROW_GAP + 1.0;
  }

  return visual_abs(from->y - to->y) <= 18.0;
}

static void write_svg_link(FILE* out, const TopologyLinkInfo* link, const VisualNode* from,
                           const VisualNode* to, int lane) {
  double start_x = from->x;
  double start_y = from->y;
  double end_x = to->x;
  double end_y = to->y;
  double icon_radius = VIS_ICON_SIZE / 2.0;
  bool straight_link = link_should_be_straight(from, to);

  if (to->x > from->x) {
    start_x += icon_radius;
    end_x -= icon_radius;
  } else if (to->x < from->x) {
    start_x -= icon_radius;
    end_x += icon_radius;
  } else if (!straight_link) {
    double side = from->x >= VIS_COLUMN_ROUTER ? 1.0 : -1.0;
    start_x += side * icon_radius;
    end_x += side * icon_radius;
  } else if (to->y > from->y) {
    start_y += icon_radius;
    end_y -= icon_radius;
  } else {
    start_y -= icon_radius;
    end_y += icon_radius;
  }

  double mid_x = (start_x + end_x) / 2.0;
  double mid_y = (start_y + end_y) / 2.0;
  if (straight_link) {
    double label_x = mid_x;
    double label_y = mid_y;
    if (from->x == to->x) {
      label_x += from->x >= VIS_COLUMN_ROUTER ? 62.0 : -62.0;
    } else {
      label_y -= 9.0;
    }

    fprintf(out,
            "  <path d=\"M %.1f %.1f L %.1f %.1f\" "
            "fill=\"none\" stroke=\"#1a8fd8\" stroke-width=\"6\" stroke-linecap=\"round\" "
            "opacity=\"0.9\"/>\n",
            start_x, start_y, end_x, end_y);
    fprintf(out,
            "  <path d=\"M %.1f %.1f L %.1f %.1f\" "
            "fill=\"none\" stroke=\"#bff0ff\" stroke-width=\"2\" stroke-linecap=\"round\" "
            "opacity=\"0.86\"/>\n",
            start_x, start_y, end_x, end_y);
    fprintf(out,
            "  <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
            "font-size=\"10\" font-weight=\"700\" fill=\"#29445d\" stroke=\"#ffffff\" "
            "stroke-width=\"3\" paint-order=\"stroke\">%s:%u - %s:%u</text>\n",
            label_x, label_y - 3.0, link->node_a, (unsigned)link->port_a, link->node_b,
            (unsigned)link->port_b);
    fprintf(out,
            "  <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
            "font-size=\"10\" fill=\"#5f7690\" stroke=\"#ffffff\" stroke-width=\"3\" "
            "paint-order=\"stroke\">delay=%ums mtu=%u</text>\n",
            label_x, label_y + 11.0, (unsigned)link->link->delay_ms, (unsigned)link->link->mtu);
    return;
  }

  double lane_offset = (double)lane * 34.0;
  double bend_x = 0.0;
  if (from->x == to->x) {
    int lane_rank = lane + 2;
    double bend_direction = from->x >= VIS_COLUMN_ROUTER ? 1.0 : -1.0;
    if (lane_rank < 0) {
      lane_rank = 0;
    }
    bend_x = bend_direction * (52.0 + (double)lane_rank * 14.0);
  }
  double bend_y = from->x == to->x ? 0.0 : lane_offset;
  double label_x = mid_x + bend_x;
  double label_y = mid_y + bend_y;
  if (from->x != to->x && label_y < VIS_TOP_MARGIN - 16.0) {
    label_y = VIS_TOP_MARGIN - 16.0;
    bend_y = label_y - mid_y;
  }

  fprintf(out,
          "  <path d=\"M %.1f %.1f C %.1f %.1f %.1f %.1f %.1f %.1f\" "
          "fill=\"none\" stroke=\"#1a8fd8\" stroke-width=\"6\" stroke-linecap=\"round\" "
          "opacity=\"0.9\"/>\n",
          start_x, start_y, mid_x + bend_x, start_y + bend_y, mid_x + bend_x, end_y + bend_y,
          end_x, end_y);
  fprintf(out,
          "  <path d=\"M %.1f %.1f C %.1f %.1f %.1f %.1f %.1f %.1f\" "
          "fill=\"none\" stroke=\"#bff0ff\" stroke-width=\"2\" stroke-linecap=\"round\" "
          "opacity=\"0.86\"/>\n",
          start_x, start_y, mid_x + bend_x, start_y + bend_y, mid_x + bend_x, end_y + bend_y,
          end_x, end_y);
  fprintf(out,
          "  <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
          "font-size=\"10\" font-weight=\"700\" fill=\"#29445d\" stroke=\"#ffffff\" "
          "stroke-width=\"3\" paint-order=\"stroke\">%s:%u - %s:%u</text>\n",
          label_x, label_y - 3.0, link->node_a, (unsigned)link->port_a, link->node_b,
          (unsigned)link->port_b);
  fprintf(out,
          "  <text x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\" "
          "font-size=\"10\" fill=\"#5f7690\" stroke=\"#ffffff\" stroke-width=\"3\" "
          "paint-order=\"stroke\">delay=%ums mtu=%u</text>\n",
          label_x, label_y + 11.0, (unsigned)link->link->delay_ms, (unsigned)link->link->mtu);
}

static int write_svg(const Topology* topology, const char* path) {
  size_t node_count = 0U;
  VisualNode* nodes = collect_nodes(topology, &node_count);
  if (nodes == NULL) {
    return magi_errno;
  }

  size_t link_count = 0U;
  TopologyLinkInfo** links = collect_links(topology, &link_count);
  if (links == NULL) {
    free(nodes);
    return magi_errno;
  }

  assign_node_positions(nodes, node_count);
  double canvas_width = VIS_COLUMN_ROUTER + (VIS_ICON_SIZE / 2.0) + VIS_RIGHT_MARGIN;
  if (canvas_width < VIS_COLUMN_HOST + VIS_RIGHT_MARGIN) {
    canvas_width = VIS_COLUMN_HOST + VIS_RIGHT_MARGIN;
  }
  double height = VIS_TOP_MARGIN + (double)(max_kind_count(nodes, node_count) - 1U) * VIS_ROW_GAP +
                  VIS_NODE_HEIGHT + 86.0;

  FILE* out = fopen(path, "w");
  if (out == NULL) {
    free(links);
    free(nodes);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  fprintf(out,
          "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.0f\" height=\"%.0f\" "
          "viewBox=\"0 0 %.0f %.0f\">\n",
          canvas_width, height, canvas_width, height);
  fputs("  <rect width=\"100%\" height=\"100%\" fill=\"#f8fafc\"/>\n", out);
  fputs("  <style>text{font-family:Arial, Helvetica, sans-serif;}</style>\n", out);
  fputs("  <text x=\"28\" y=\"38\" font-size=\"22\" font-weight=\"700\" "
        "fill=\"#1f2933\">Magi System Topology</text>\n",
        out);
  fprintf(out,
          "  <text x=\"28\" y=\"60\" font-size=\"12\" fill=\"#59636d\">"
          "hosts=%zu switches=%zu routers=%zu links=%zu</text>\n",
          topology_count_nodes_of_kind(topology, TOPOLOGY_NODE_HOST),
          topology_count_nodes_of_kind(topology, TOPOLOGY_NODE_SWITCH),
          topology_count_nodes_of_kind(topology, TOPOLOGY_NODE_ROUTER), link_count);
  fputs("  <text x=\"150\" y=\"110\" font-size=\"13\" font-weight=\"700\" "
        "text-anchor=\"middle\" fill=\"#1f2933\">HOSTS</text>\n",
        out);
  fputs("  <text x=\"460\" y=\"110\" font-size=\"13\" font-weight=\"700\" "
        "text-anchor=\"middle\" fill=\"#1f2933\">SWITCHES</text>\n",
        out);
  fputs("  <text x=\"770\" y=\"110\" font-size=\"13\" font-weight=\"700\" "
        "text-anchor=\"middle\" fill=\"#1f2933\">ROUTERS</text>\n",
        out);

  for (size_t index = 0U; index < link_count; ++index) {
    const TopologyLinkInfo* link = links[index];
    const VisualNode* from = find_visual_node(nodes, node_count, link->node_a);
    const VisualNode* to = find_visual_node(nodes, node_count, link->node_b);
    if (from != NULL && to != NULL) {
      int lane = (int)(index % 5U) - 2;
      write_svg_link(out, link, from, to, lane);
    }
  }

  for (size_t index = 0U; index < node_count; ++index) {
    write_svg_node(out, &nodes[index]);
  }

  fputs("</svg>\n", out);
  int close_status = fclose(out);
  free(links);
  free(nodes);

  if (close_status != 0) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return MAGI_OK;
}

static void write_dot_node(FILE* out, const TopologyNodeInfo* info) {
  const char* shape = info->kind == TOPOLOGY_NODE_ROUTER ? "hexagon" : "box";

  fputs("  \"", out);
  dot_write_escaped(out, info->node->name);
  fprintf(out, "\" [shape=%s, style=\"rounded,filled\", fillcolor=\"%s\", color=\"%s\", label=\"",
          shape, color_for_kind(info->kind), stroke_for_kind(info->kind));
  dot_write_escaped(out, info->node->name);
  fputs("\\n", out);
  dot_write_escaped(out, topology_kind_name(info->kind));
  if (info->kind == TOPOLOGY_NODE_HOST && info->ip_address[0] != '\0') {
    fputs("\\n", out);
    dot_write_escaped(out, info->ip_address);
  } else if (info->kind == TOPOLOGY_NODE_SWITCH && info->num_ports > 0U) {
    fprintf(out, "\\nports=%u", (unsigned)info->num_ports);
  }
  fputs("\"];\n", out);
}

static int write_dot(const Topology* topology, const char* path) {
  size_t node_count = 0U;
  VisualNode* nodes = collect_nodes(topology, &node_count);
  if (nodes == NULL) {
    return magi_errno;
  }

  size_t link_count = 0U;
  TopologyLinkInfo** links = collect_links(topology, &link_count);
  if (links == NULL) {
    free(nodes);
    return magi_errno;
  }

  FILE* out = fopen(path, "w");
  if (out == NULL) {
    free(links);
    free(nodes);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  fputs("graph magi_topology {\n", out);
  fputs("  graph [rankdir=LR, bgcolor=\"#f8fafc\"];\n", out);
  fputs("  node [fontname=\"Arial\"];\n", out);
  fputs("  edge [fontname=\"Arial\", color=\"#69737d\"];\n", out);

  for (size_t index = 0U; index < node_count; ++index) {
    write_dot_node(out, nodes[index].info);
  }

  for (size_t index = 0U; index < link_count; ++index) {
    const TopologyLinkInfo* link = links[index];
    fputs("  \"", out);
    dot_write_escaped(out, link->node_a);
    fputs("\" -- \"", out);
    dot_write_escaped(out, link->node_b);
    fprintf(out, "\" [label=\"%u:%u\\ndelay=%ums mtu=%u\"];\n", (unsigned)link->port_a,
            (unsigned)link->port_b, (unsigned)link->link->delay_ms, (unsigned)link->link->mtu);
  }

  fputs("}\n", out);
  int close_status = fclose(out);
  free(links);
  free(nodes);

  if (close_status != 0) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return MAGI_OK;
}

int topology_visualize(const Topology* topology, const char* filename) {
  if (topology == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* path = filename != NULL && filename[0] != '\0' ? filename : "topology.svg";

  if (has_suffix(path, ".svg")) {
    return write_svg(topology, path);
  }
  if (has_suffix(path, ".dot")) {
    return write_dot(topology, path);
  }

  magi_errno = MAGI_ERR_BADARGS;
  return MAGI_ERR_BADARGS;
}
