ref "std/log"
ref "ncurses"

log: Log("ncurses")

scr = initscr()
h = getmaxy(scr)
w = getmaxx(scr)
move(h/2-1, w/2-1)
printw("hello, ox!")
refresh()
getch()
endwin()