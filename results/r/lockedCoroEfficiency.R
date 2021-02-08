x <- c(2,4,8,16)
y1 <- c(80.6,  82.4,  49.9,  20.7)
y2 <- c(91.4,  91.3,  87.9,  56.9)
y3 <- c(93.4,  94.2,  91.9,  81.6)
y4 <- c(95.5,  95.5,  94.1,  90.8)
y5 <- c(96.4,  96.5,  95.4,  93.5)
y6 <- c(97.1,  97.3,  96.3,  94.9)
y7 <- c(97.7,  97.7,  97.0,  95.9)
y8 <- c(98.2,  98.1,  97.5,  96.6)
y9 <- c(98.5,  98.5,  97.9,  97.0)
y10 <-c(98.7,  98.6,  98.1,  97.5)

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

title(main="(Locked) Coroutine efficiency wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="(%) Efficiency", col.lab=rgb(0,0,0))

legend(1, 45, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);