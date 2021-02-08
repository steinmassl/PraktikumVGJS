x <- c(2,4,8,16)
y8   <- c(1.93297,   3.56754,   5.82226,   5.83241)
y16  <- c(1.95771,   3.6885 ,   5.44815,   7.59233)
y32  <- c(1.96865,   3.78403,   5.75103,   9.06394)
y64  <- c(1.9536 ,   3.82813,   6.2303 ,   9.31376)
y128 <- c(1.97885,   3.86461,   6.30605,   9.47818)
y256 <- c(1.80736,   3.81053,   6.31264,   9.3134 )

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

title(main="(Lock-free) MCTS Function Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("    8 Trees","  16 Trees","  32 Trees","  64 Trees","128 Trees","256 Trees"), 
   col=c("black","brown","purple", "blue", "red", "green"), 
	pch=21, lty=1, cex=1, lwd=2);