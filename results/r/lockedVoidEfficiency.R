x <- c(2,4,8,16)
y1 <- c(79.0,  82.7,  54.8,  12.1)
y2 <- c(89.3,  91.1,  86.6,  38.0)
y3 <- c(92.0,  94.0,  90.7,  84.2)
y4 <- c(94.3,  95.7,  92.9,  90.8)
y5 <- c(95.4,  96.4,  94.5,  92.8)
y6 <- c(96.1,  97.0,  95.4,  94.0)
y7 <- c(96.7,  97.6,  96.0,  94.9)
y8 <- c(97.2,  98.0,  96.5,  95.4)
y9 <- c(97.6,  98.1,  96.9,  96.0)
y10 <-c(97.8,  98.3,  97.3,  96.4)

g_yrange <- range(1, 100)
g_xrange <- range(1, 16)

plot(x, y1, lwd=2, type="o", col="black",
ylim=g_yrange, xlim=g_xrange, axes=FALSE, ann=FALSE)

lines(x, y2, lwd=2, type="o", col="brown")
lines(x, y3, lwd=2, type="o", col="purple")
lines(x, y4, lwd=2, type="o", col="blue")
lines(x, y5, lwd=2, type="o", col="red")
lines(x, y6, lwd=2, type="o", col="green")
lines(x, y7, lwd=2, type="o", col="orange")
lines(x, y8, lwd=2, type="o", col="yellow")
lines(x, y9, lwd=2, type="o", col="pink")

axis(1, las=1, at=1:16)
# par(yaxp = c(0,100,5))
axis(2, las=1, at=0:100 * 5)

grid(nx=NA, ny=NULL, col="gray")

box()

title(main="(Locked) Function pointer efficiency wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="(%) Efficiency", col.lab=rgb(0,0,0))

legend(1, 45, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);