x <- c(2,4,8,16)
y1 <- c(1.5522 ,   3.27354,   4.2672 ,   1.11261)
y2 <- c(1.77663,   3.62537,   6.84546,   5.70764)
y3 <- c(1.82906,   3.7508 ,   7.24941,   11.3122)
y4 <- c(1.88578,   3.81168,   7.43851,   14.5113)
y5 <- c(1.90426,   3.84563,   7.53412,   14.8362)
y6 <- c(1.91906,   3.87497,   7.615  ,   15.0346)
y7 <- c(1.92984,   3.89525,   7.67697,   15.1735)
y8 <- c(1.9397 ,   3.90932,   7.71627,   15.281 )
y9 <- c(1.94764,   3.91765,   7.75029,   15.3684)
y10 <-c(1.95304,   3.92493,   7.76288,   15.4215)

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

title(main="(Locked) Function Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("1us","2us","3us","4us","5us", "6us", "7us", "8us","9us"), 
   col=c("black","brown","purple", "blue", "red", "green", "orange", "yellow", "pink"), 
	pch=21, lty=1, cex=1, lwd=2);