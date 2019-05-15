
unsigned long vrniPomnilnik() {
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

char* vrniVelikost(long bytes) {
  static char velikost_enota[8];
  char enota[3] = "B";
  if (bytes >= 1000) {
    bytes /= 1000; strcpy(enota, "kB");
  }
  if (bytes >= 1000) {
    bytes /= 1000; strcpy(enota, "MB");
  }
  if (bytes >= 1000) {
    bytes /= 1000; strcpy(enota, "GB");
  }
  sprintf(velikost_enota, "%d %s", (unsigned char) bytes, enota);
  return velikost_enota;
}

char* je_sta_so(unsigned char stevilo) {
  static char beseda[4];
  if (stevilo == 2) sprintf(beseda, "sta");
  else if (stevilo == 3 || stevilo == 4) sprintf(beseda, "so");
  else sprintf(beseda, "je");
  return beseda;
}

char* d_h_m_s(int s) {
  static char cas[23];
  int d = s / 86400; s %= 86400;
  int h = s / 3600; s %= 3600;
  int m = s / 60; s %= 60;
  if (d > 0)      sprintf(cas, "%dd %dh %dm %ds", d, h, m, s);
  else if (h > 0) sprintf(cas, "%dh %dm %ds", h, m, s);
  else if (m > 0) sprintf(cas, "%dm %ds", m, s);
  else            sprintf(cas, "%ds", s);
  return cas;
}
