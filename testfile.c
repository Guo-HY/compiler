int main() {
    const int a1 = 1;
    const int _bc = -2, _de = 3;
    const int fff = +4, ggg = 5, hhh = 6;
    int x1 = 111;
    int x2 = 123 + 234;
    int x3 = 6 * (5 + 3) - 10;
    int x4;
    x4 = x1 + x2 * 3;
    int x5, x6;
    int x7;

    int y1, y2, y3, y4, y5, y6, y7, y8;

    x5 = getint();
    x6 = getint();
    x7 = getint();
    x8 = getint();
    x9 = getint();
    x10 = getint();

    printf("20373184\n");
    printf("Wow, you will begin test your program!\n");
    printf("%d\n", 1 + 2);
    printf("%d\n", ggg * hhh);

    y1 = (a1 + _bc) * _de;
    y2 = x2 % x1 + ggg;
    y3 = x4 * x3 - fff;
    y4 = x5 / 2 * hhh;
    y5 = x6 * (1 + x7) % 3;
    y6 = -x8;
    y7 = 6 + x9;
    printf("%d\n", y1);
    printf("%d\n", y2);
    printf("%d\n", y3);
    printf("%d\n", y4);
    printf("%d\n", y5);
    printf("%d-%d\n", y6, y7);
    return 0;
}