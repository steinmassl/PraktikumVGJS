x <- c(2,4,8,16)
y1 <- c(67.8,  71.9,  13.9,   4.6)
y2 <- c(82.4,  84.5,  57.5,   9.2)
y3 <- c(86.0,  89.4,  83.8,  14.3)
y4 <- c(90.8,  92.1,  87.8,  19.8)
y5 <- c(92.4,  93.7,  90.0,  25.9)
y6 <- c(93.7,  94.7,  91.6,  33.8)
y7 <- c(94.6,  95.6,  92.9,  59.7)
y8 <- c(95.3,  96.2,  93.8,  91.7)
y9 <- c(95.8,  96.7,  94.5,  92.7)
y10 <-c(96.3,  97.1,  95.0,  93.6)

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

title(main="(Lock-free) Function efficiency wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="(%) Efficiency", col.lab=rgb(0,0,0))

legend(1, 45, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);