#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
using namespace std;
#define REP(i, n) for (int i = 0; (i) < (int)(n); ++ (i))
#define REP3(i, m, n) for (int i = (m); (i) < (int)(n); ++ (i))

struct point_t {
    int y, x;
};

constexpr int BITS = 8;

struct config_t {
    point_t switch_input, switch_output, switch_halt;
    array<point_t, BITS> input, output;
};

struct state_t {
    point_t size, shift;
    vector<bool> field;
};

void skip_comment(FILE *fh) {
    // NOTE: the '#' or '!' should be appeared only at the beggening of lines
    while (true) {
        char c;
        if (fscanf(fh, " %c", &c) != 1) assert (false);
        if (not (c == '#' or c == '!')) {
            ungetc(c, fh);
            break;
        }
        while (true) {
            int c = fgetc(fh);
            if (c == EOF or c == '\n') break;  // NOTE: non-ASCII may causes errors
        }
    }
}

int read_point(FILE *fh, point_t & p) {
    int value = fscanf(fh, "%d%d", &p.y, &p.x);
    skip_comment(fh);
    return value;
}

void read_program(FILE *fh, config_t & config, state_t & state) {
    // TODO: err msg
    skip_comment(fh);
    int version;
    if (fscanf(fh, "LIFE%d", &version) != 1) assert (false);
    skip_comment(fh);
    if (version != 1) assert (false);
    int h, w;
    if (fscanf(fh, "%d%d", &h, &w) != 2) assert (false);
    skip_comment(fh);
    if (read_point(fh, config.switch_input) != 2) assert (false);
    REP (i, BITS) if (read_point(fh, config.input[i]) != 2) assert (false);
    if (read_point(fh, config.switch_output) != 2) assert (false);
    REP (i, BITS) if (read_point(fh, config.output[i]) != 2) assert (false);
    if (read_point(fh, config.switch_halt) != 2) assert (false);
    state.size.y = h + 2;
    state.size.x = w + 2;
    state.shift.y = 1;
    state.shift.x = 1;

    // NOTE: this follows http://www.conwaylife.com/wiki/Plaintext
    state.field.resize((h + 2) * (w + 2));
    REP (y, h) {
        skip_comment(fh);
        REP (x, w) {
            char c;
            if (fscanf(fh, " %c", &c) != 1) assert (false);
            if (not (c == '.' or c == 'O')) assert (false);
            state.field[(1 + y) * state.size.x + (1 + x)] = (c == 'O');
        }
    }
}

state_t extend_field(state_t const & prv) {
    // NOTE: this should extend only for required axis, not always both
    state_t nxt;
    nxt.size.y = 3 * prv.size.y + 4;
    nxt.size.x = 3 * prv.size.x + 4;
    nxt.shift.y = 2 + prv.size.y + prv.shift.y;
    nxt.shift.x = 2 + prv.size.x + prv.shift.x;
    nxt.field.resize(nxt.size.y * nxt.size.x);
    REP (y, prv.size.y) {
        REP (x, prv.size.x) {
            int ny = 2 + prv.size.y + y;
            int nx = 2 + prv.size.x + x;
            nxt.field[ny * nxt.size.x + nx] = prv.field[y * prv.size.x + x];
        }
    }
    return nxt;
}

void swap_state(state_t & a, state_t & b) {
    swap(a.size, b.size);
    swap(a.shift, b.shift);
    a.field.swap(b.field);
}

bool at(state_t const & state, int y, int x) {
    return state.field[y * state.size.x + x];
}

void execute_step(state_t & state) {
    // check sentinels and extend if required
    bool found = false;
    REP (x, state.size.x) {
        if (at(state, 1, x) or at(state, state.size.y - 2, x)) {
            found = true;
            break;
        }
    }
    if (not found) {
        REP (y, state.size.y) {
            if (at(state, y, 1) or at(state, y, state.size.x - 2)) {
                found = true;
                break;
            }
        }
    }
    if (found) {
        state = extend_field(state);
    }

    // update cells
    vector<bool> updated(state.size.y * state.size.x);
    REP3 (y, 1, state.size.y - 1) {
        REP3 (x, 1, state.size.x - 1) {
            int cnt = 0;
            REP3 (ny, y - 1, y + 2) {
                REP3 (nx, x - 1, x + 2) {
                    cnt += at(state, ny, nx);
                }
            }
            updated[y * state.size.x + x] = (cnt == 3 or (at(state, y, x) and cnt == 4));
        }
    }
    state.field.swap(updated);
}

bool at_shifted(state_t const & state, point_t p) {
    int y = state.shift.y + p.y;
    int x = state.shift.x + p.x;
    if (not (0 <= y and y < state.size.y and 0 <= x and x < state.size.x)) return false;
    return state.field[y * state.size.x + x];
}

char read_value_from_cells(state_t const & state, array<point_t, BITS> const & p) {
    unsigned c = 0;
    REP (i, BITS) {
        if (at_shifted(state, p[i])) {
            c |= 1u << i;
        }
    }
    return c;
}

void write_value_to_cells(state_t & state, array<point_t, BITS> const & p, char c) {
    REP (i, BITS) {
        state.field[(p[i].y - state.shift.y) * state.size.x + (p[i].x - state.shift.x)] = (bool)(c & (1 << i));
    }
}

const char *usage = "Usage: %s FILE\n";
int main(int argc, const char **argv) {
    // read options
    if (argc != 2) {
        fprintf(stderr, usage, argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *path = argv[1];

    // read the program
    FILE *fh = fopen(path, "r");
    config_t config;
    state_t state;
    read_program(fh, config, state);
    extend_field(state);

    // execute the program
    while (true) {
        execute_step(state);

        // NOTE: should call oracles if cells are living at initial states?
        // NOTE: should the order of switchs be implementation defined?
        if (at_shifted(state, config.switch_input)) {
            int c = getchar();
            if (c == EOF) {
                // TODO: what should I do?
            } else {
                write_value_to_cells(state, config.input, c);
            }
        }
        if (at_shifted(state, config.switch_output)) {
            char c = read_value_from_cells(state, config.output);
            putchar(c);
            fflush(stdout);
        }
        if (at_shifted(state, config.switch_halt)) {
            break;
        }
    }
    return EXIT_SUCCESS;
}
