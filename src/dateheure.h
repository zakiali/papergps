/* ------------------------------------------------------------------------ */
/* -- provide date/hour as a formated string (with ms) and double        -- */
/* ------------------------------------------------------------------------ */
double dateheurec(char *dateymd, char *heurehms);

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as a compact formated string and double          -- */
/* ------------------------------------------------------------------------ */
double dateheure(char *dateymd, char *heurehms);

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as formated (suitable to men) and double         -- */
/* ------------------------------------------------------------------------ */
double dateheureimpr(char *dateymd, char *heurehms, char *tuhms);

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as integers                                      -- */
/* ------------------------------------------------------------------------ */
double dateheureint(int *jy, int *jm, int *jd, int *jhr, int *jmn, int *jsc);

/* ------------------------------------------------------------------------ */
/* -- provide formated date (in one string) and fraction of day (us)     -- */
/* ------------------------------------------------------------------------ */
long datehour(char *datehour);

/* ------------------------------------------------------------------------ */
/* -- provide formated date (in one string) and Julian Date              -- */
/* ------------------------------------------------------------------------ */
double jdcur(void);
