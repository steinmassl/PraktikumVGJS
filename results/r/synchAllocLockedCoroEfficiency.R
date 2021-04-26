x <- c(2,4,8,16)
y1 <- c(77.1,  67.8,  44.4,  17.1)
y2 <- c(87.9,  82.1,  69.2,  49.5)
y3 <- c(91.6,  86.9,  77.5,  62.1)
y4 <- c(94.3,  91.2,  84.1,  72.9)
y5 <- c(95.2,  92.6,  86.2,  76.4)
y6 <- c(95.8,  93.1,  87.6,  78.4)
y7 <- c(96.2,  93.4,  88.5,  80.0)
y8 <- c(97.9,  96.7,  93.5,  87.3)
y9 <- c(98.2,  97.3,  94.2,  89.3)
y10 <-c(98.3,  97.6,  95.0,  89.9)

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