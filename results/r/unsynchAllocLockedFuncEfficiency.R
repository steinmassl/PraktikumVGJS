x <- c(2,4,8,16)
y1 <- c(77.8,  79.8,  52.1,   7.4)
y2 <- c(87.8,  89.4,  84.1,  32.7)
y3 <- c(91.1,  92.8,  88.9,  71.3)
y4 <- c(93.9,  94.6,  91.2,  88.1)
y5 <- c(95.1,  95.5,  93.2,  89.9)
y6 <- c(95.8,  96.3,  94.1,  92.1)
y7 <- c(96.4,  96.9,  95.0,  93.2)
y8 <- c(96.9,  97.4,  95.7,  94.0)
y9 <- c(97.2,  97.6,  96.2,  94.7)
y10 <-c(97.6,  97.1,  96.6,  95.2)

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