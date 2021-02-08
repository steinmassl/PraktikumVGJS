x <- c(2,4,8,16)
y8   <- c(1.93506,   3.41352,   5.669  ,   5.66068)
y16  <- c(1.94451,   3.67224,   5.89568,   8.03844)
y32  <- c(1.9525 ,   3.67424,   5.87698,   9.01553)
y64  <- c(1.94475,   3.78263,   6.15778,   9.26096)
y128 <- c(1.98673,   3.81407,   6.15722,   9.37551)
y256 <- c(1.9525 ,   3.8964 ,   6.26929,   9.45565)

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

title(main="(Locked) MCTS Function Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("    8 Trees","  16 Trees","  32 Trees","  64 Trees","128 Trees","256 Trees"), 
   col=c("black","brown","purple", "blue", "red", "green"), 
	pch=21, lty=1, cex=1, lwd=2);