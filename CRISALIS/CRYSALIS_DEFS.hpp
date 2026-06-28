#pragma once
#include <bit>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#define BASEDATATYPE uint16_t  // MUST BE UNSIGNED
#define S_BASEDATATYPE int16_t // MUST BE SIGNED
#define RES_DATATYPE int64_t
#define COMPDATATYPE uint8_t

const bool DEBUG = false;

// CAN BE CHANGED~

const BASEDATATYPE ELIM =
    256; // number of coefficients (MUST BE MULTIPLE OF 8) FINAL = 256
const BASEDATATYPE clim = 4095; // Maximum val of coefficient FINAL = 4095
const BASEDATATYPE SMALL_INT = 3;
const BASEDATATYPE BASEDATA_MAX = 0 - 1;

const BASEDATATYPE BASE_W_H = 2;

// DONT TOUCH
const BASEDATATYPE CLIM_BITS = std::bit_width(clim);

const BASEDATATYPE COMPRESSION = CLIM_BITS / 2;

const BASEDATATYPE SEED_LEN = ELIM / 8;

const BASEDATATYPE COMP_DATUM_LEN = sizeof(COMPDATATYPE) * 8;

const BASEDATATYPE POLLYN_SIZE = (sizeof(BASEDATATYPE) * ELIM);

const BASEDATATYPE COMP_POLLYN_SIZE = (sizeof(COMPDATATYPE) * ELIM) + 1;

const BASEDATATYPE DIV_SHIFT = 32 - sizeof(COMPDATATYPE) * 8;

// BIT OPERATIONS
template <typename DATATYPE> static int GET_BIT(DATATYPE data, int i) {
  DATATYPE bit = 0;
  bit =
      data & ((DATATYPE)1 << i); // creates a number that has 1 at the position
                                 // that we want, we then and it with the data
                                 // to get wether there is also a 1 there
  if (bit) {
    return (1);
  }
  return (0);
}

template <typename DATATYPE> static std::string PRINT_BITS(DATATYPE datum) {
  std::string RetVal = "";
  for (int i = 0; i < sizeof(DATATYPE) * 8; i++) {
    RetVal = std::to_string(GET_BIT(datum, i)) + RetVal;
  }
  return (RetVal);
}

static void PRINT_BIT_STRING(std::string DATA) {
  for (int i = 0; i < DATA.length(); i++) {
    std::cout << PRINT_BITS(DATA[i]) << " : ";
    if (!((i + 1) % 4)) {
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
}

// PROGRESS BAR

// VARIABLES
static BASEDATATYPE CURRENT = 0;
static BASEDATATYPE PERCENT = 0;
static BASEDATATYPE END = 100;
static double PERCENT_PER_INCRAMENT = 1;
static std::string PRE_TEXT = "";
static std::string PRINT_STRING = "";
static bool BAR_STARTED = false;
bool PRINTING_TO_TERM = true;

// FUNCTIONS

#ifdef _WIN32
#include <windows.h>
static int terminal_width() {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
  return -1;
}

#else
#include <sys/ioctl.h>
#include <unistd.h>
static int terminal_width() {
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
    return w.ws_col;
  }
  return -1;
}

#endif
#include <fstream>
static bool is_terminal(std::ostream &os) {
  return dynamic_cast<std::filebuf *>(os.rdbuf()) != nullptr;
}

static void PROGRESS_BAR(std::string post_text = "") {
  CURRENT++;
  PERCENT = (int)(PERCENT_PER_INCRAMENT * CURRENT);
  if (terminal_width() != -1 && PRINTING_TO_TERM) {
    if (BAR_STARTED) {
      std::cout << "\033[" +
                       std::to_string(
                           (PRINT_STRING.length() / terminal_width()) + 1) +
                       "A\r";
    } else {
      BAR_STARTED = true;
    }

    PRINT_STRING = PRE_TEXT + "WORKING : [";
    for (int i = 0; i < 100; i++) {
      if (i < PERCENT) {
        PRINT_STRING += "#";
      } else {
        PRINT_STRING += " ";
      }
    }
    PRINT_STRING += ("]" + std::to_string(CURRENT) + "/" + std::to_string(END) +
                     "|" + post_text + "\n");

    std::cout << PRINT_STRING << std::flush;
  }

  if (CURRENT == END) {
    PRINT_STRING = "";
    std::cout << std::endl;
    BAR_STARTED = false;
  }
}

static void START_PROGRESS_BAR(int incraments = 100,
                               std::string pre_text = "") {
  END = incraments;
  PRE_TEXT = pre_text;
  CURRENT = -1;
  PERCENT = 0;
  PERCENT_PER_INCRAMENT = (double)100 / incraments;
  BAR_STARTED = false;
  PROGRESS_BAR();
}
