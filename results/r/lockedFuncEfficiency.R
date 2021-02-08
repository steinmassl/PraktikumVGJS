x <- c(2,4,8,16)
y1 <- c(77.6,  81.8,  53.3,   7.0)
y2 <- c(88.8,  90.6,  85.6,  35.7)
y3 <- c(91.5,  93.8,  90.6,  70.7)
y4 <- c(94.3,  95.3,  93.0,  90.7)
y5 <- c(95.2,  96.1,  94.2,  92.2)
y6 <- c(96.0,  96.9,  95.2,  94.0)
y7 <- c(96.5,  97.4,  96.0,  94.8)
y8 <- c(97.0,  97.7,  96.5,  95.5)
y9 <- c(97.4,  97.9,  96.9,  96.1)
y10 <-c(97.7,  98.1,  97.0,  96.4)

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

title(main="(Locked) Function efficiency wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="(%) Efficiency", col.lab=rgb(0,0,0))

legend(1, 45, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);