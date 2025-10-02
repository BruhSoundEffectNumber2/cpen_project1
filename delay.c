/* delay n milliseconds */
void delayMs(int n) {
    int i, j;
    for(i = 0 ; i< n; i++)
        for(j = 0; j < 6265; j++) {} /* do nothing for 1 ms */
}
