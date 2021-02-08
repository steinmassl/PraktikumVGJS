x <- c(2,4,8,16)
y1 <- c(70.1,  74.0,  17.6,   5.1)
y2 <- c(83.6,  85.9,  80.1,  10.6)
y3 <- c(89.0,  90.6,  86.5,  17.4)
y4 <- c(91.8,  93.1,  89.4,  28.5)
y5 <- c(93.7,  94.6,  92.1,  82.8)
y6 <- c(95.0,  95.7,  93.6,  91.0)
y7 <- c(95.9,  96.5,  94.7,  92.8)
y8 <- c(96.6,  97.0,  95.5,  93.8)
y9 <- c(97.1,  97.5,  96.1,  94.7)
y10 <-c(97.5,  97.8,  96.5,  95.4)

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

title(main="(Lock-free) Coroutine efficiency wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="(%) Efficiency", col.lab=rgb(0,0,0))

legend(1, 45, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);