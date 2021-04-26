x <- c(2,4,8,16)
y1 <- c(77.9,  79.8,  54.8,   7.0)
y2 <- c(88.0,  89.5,  83.8,  34.8)
y3 <- c(91.0,  92.8,  88.7,  73.4)
y4 <- c(93.8,  94.5,  91.4,  88.0)
y5 <- c(95.1,  95.6,  93.0,  90.5)
y6 <- c(95.7,  96.3,  94.2,  92.1)
y7 <- c(96.4,  97.0,  95.2,  93.1)
y8 <- c(96.9,  97.2,  95.7,  94.0)
y9 <- c(97.2,  97.7,  96.2,  94.0)
y10 <-c(97.5,  97.9,  96.5,  95.3)

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