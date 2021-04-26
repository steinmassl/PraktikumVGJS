x <- c(2,4,8,16)
y1 <- c(76.9,  68.6,  44.6,  17.1)
y2 <- c(87.4,  82.3,  68.3,  50.4)
y3 <- c(91.2,  87.3,  76.4,  62.3)
y4 <- c(93.8,  91.7,  81.9,  73.8)
y5 <- c(94.7,  92.8,  85.1,  76.8)
y6 <- c(95.7,  93.2,  87.4,  79.1)
y7 <- c(96.3,  93.6,  88.6,  80.4)
y8 <- c(97.1,  97.0,  91.1,  88.7)
y9 <- c(97.5,  97.7,  92.2,  90.2)
y10 <-c(97.7,  97.9,  92.1,  90.3)

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