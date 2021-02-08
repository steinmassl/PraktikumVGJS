x <- c(2,4,8,16)
y8   <- c(1.84299,   3.3318 ,   5.87806,   6.04839)
y16  <- c(1.91722,   3.52284,   5.87936,   8.52399)
y32  <- c(1.91048,   3.72823,   5.89921,   9.15068)
y64  <- c(1.93033,   3.7783 ,   6.26888,   9.42344)
y128 <- c(1.94967,   3.81896,   6.33285,   9.43273)
y256 <- c(2.01048,   3.7727 ,   6.33835,   9.43656)

g_range <- range(1, 16)

plot(x, y8, lwd=2, type="o", col="black",
ylim=g_range, xlim=g_range, axes=FALSE, ann=FALSE)

lines(x, y16,  lwd=2, type="o", col="brown")
lines(x, y32,  lwd=2, type="o", col="purple")
lines(x, y64,  lwd=2, type="o", col="blue")
lines(x, y128, lwd=2, type="o", col="red")
lines(x, y256, lwd=2, type="o", col="green")

axis(1, las=1, at=1:16)
axis(2, las=1, at=1:16)

grid(nx=NA, ny=NULL, col="gray")

box()

title(main="(Lock-free) MCTS Coroutine Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("    8 Trees","  16 Trees","  32 Trees","  64 Trees","128 Trees","256 Trees"), 
   col=c("black","brown","purple", "blue", "red", "green"), 
	pch=21, lty=1, cex=1, lwd=2);