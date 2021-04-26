x <- c(2,4,8,16)
y1 <- c(78.1,  81.0,  53.5,  13.1)
y2 <- c(88.7,  90.4,  85.0,  37.5)
y3 <- c(91.5,  93.4,  89.6,  82.9)
y4 <- c(94.1,  95.0,  91.9,  88.2)
y5 <- c(95.3,  96.0,  93.8,  91.4)
y6 <- c(96.1,  96.8,  94.7,  92.9)
y7 <- c(96.7,  97.3,  95.7,  93.9)
y8 <- c(97.2,  97.7,  96.1,  94.9)
y9 <- c(97.5,  97.9,  96.5,  95.3)
y10 <-c(97.8,  98.2,  97.0,  95.8)

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