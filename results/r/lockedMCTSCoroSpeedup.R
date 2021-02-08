x <- c(2,4,8,16)
y8   <- c(1.88395,   3.27738,   5.84331,   5.68237)
y16  <- c(1.9452 ,   3.45378,   5.93927,   8.16097)
y32  <- c(1.93992,   3.66041,   5.84557,   8.94699)
y64  <- c(1.95609,   3.7908 ,   6.12613,   9.20928)
y128 <- c(1.9796 ,   3.82533,   6.13116,   9.33996)
y256 <- c(1.96783,   3.75641,   6.30394,   9.38823)

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

title(main="(Locked) MCTS Coroutine Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("    8 Trees","  16 Trees","  32 Trees","  64 Trees","128 Trees","256 Trees"), 
   col=c("black","brown","purple", "blue", "red", "green"), 
	pch=21, lty=1, cex=1, lwd=2);