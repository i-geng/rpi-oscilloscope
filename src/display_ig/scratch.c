
def convert_data_to_ascii(float data) {
  char buffer[10];
  snprintk(buffer, sizeof(buffer), "%.2f  ", data);

  for (size_t i = 1; i <= strlen(buffer); i++) {
    multi_display_draw_character(50 - 5 * i,
                                 32,
                                 buffer[strlen(buffer) - i], COLOR_WHITE);
  }
}

  
