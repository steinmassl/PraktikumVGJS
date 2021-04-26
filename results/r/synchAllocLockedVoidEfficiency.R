x <- c(2,4,8,16)
y1 <- c(78.1,  80.6,  54.6,  14.3)
y2 <- c(88.7,  90.3,  84.6,  38.9)
y3 <- c(91.5,  93.4,  89.4,  82.5)
y4 <- c(93.9,  95.0,  92.1,  88.7)
y5 <- c(95.0,  96.0,  93.7,  91.7)
y6 <- c(95.8,  96.8,  94.7,  92.9)
y7 <- c(96.5,  97.3,  95.6,  94.1)
y8 <- c(97.0,  97.7,  96.1,  94.6)
y9 <- c(97.4,  97.9,  96.5,  95.2)
y10 <-c(97.7,  98.1,  97.0,  95.8)

g_yrange <- range(1, 100)
g_xrange <- range(1, 16)

plot(x, y1, lwd=2, type="l", col="black",
ylim=g_yrange, xlim=g_xrange, axes=FALSE, ann=FALSE)

lines(x, y2, lwd=2, col="brown")
lines(x, y3, lwd=2, col="purple")
lines(x, y4, lwd=2, col="blue")
lines(x, y5, lwd=2, col="red")
lines(x, y6, lwd=2, col="green")
lines(x, y7, lwd=2, col="orange")
lines(x, y8, lwd=2, col="yellow")
lines(x, y9, lwd=2, col="pink")

axis(1, las=1, at=1:16,      cex.axis=1.1)
axis(2, las=1, at=0:100 * 5, cex.axis=1.1)

grid(nx=NA, ny=NULL, col="gray")

box()

title(xlab="(#) Threads",    cex.lab=1.5)
title(ylab="(%) Efficiency", cex.lab=1.5)

legend(1, 60, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1.4, lwd=2);