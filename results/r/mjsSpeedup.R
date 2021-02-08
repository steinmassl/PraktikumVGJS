x <- c(2,4,8,16)
y1 <- c(1.71146,   3.30966,   6.16041,   1.05249)
y2 <- c(1.85398,   3.68447,   7.10339,   2.06187)
y3 <- c(1.90295,   3.79711,   7.21379,   3.21537)
y4 <- c(1.69293,   3.40713,   6.78199,   4.15553)
y5 <- c(1.60761,   3.28375,   6.51114,   5.21268)
y6 <- c(1.68109,   3.37709,   6.70325,   7.59273)
y7 <- c(1.72036,   3.45266,   6.8279 ,   11.8415)
y8 <- c(1.75139,   3.51737,   6.9254 ,   12.5073)
y9 <- c(1.77634,   3.56245,   7.01721,   12.9448)
y10 <-c(1.79628,   3.60105,   7.12877,   13.2568)

g_range <- range(1, 16)

plot(x, y1, lwd=2, type="o", col="black",
ylim=g_range, xlim=g_range, axes=FALSE, ann=FALSE)

lines(x, y2, lwd=2, type="o", col="brown")
lines(x, y3, lwd=2, type="o", col="purple")
lines(x, y4, lwd=2, type="o", col="blue")
lines(x, y5, lwd=2, type="o", col="red")
lines(x, y6, lwd=2, type="o", col="green")
lines(x, y7, lwd=2, type="o", col="orange")
lines(x, y8, lwd=2, type="o", col="yellow")
lines(x, y9, lwd=2, type="o", col="pink")

axis(1, las=1, at=1:16)
axis(2, las=1, at=1:16)

grid(nx=NA, ny=NULL, col="gray")

box()

title(main="Minimal Job System Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("1us","2us","3us","4us","5us", "6us", "7us", "8us","9us"), 
   col=c("black","brown","purple", "blue", "red", "green", "orange", "yellow", "pink"), 
	pch=21, lty=1, cex=1, lwd=2);