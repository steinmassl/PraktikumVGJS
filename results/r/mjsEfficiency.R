x <- c(2,4,8,16)
y1 <- c(85.6,  82.7,  77.0,   6.6)
y2 <- c(92.7,  92.1,  88.8,  12.9)
y3 <- c(95.1,  94.9,  90.2,  20.1)
y4 <- c(85.6,  85.2,  84.8,  26.0)
y5 <- c(80.4,  82.1,  81.4,  32.6)
y6 <- c(84.1,  84.4,  83.8,  47.5)
y7 <- c(86.0,  86.3,  85.3,  74.0)
y8 <- c(87.6,  87.9,  86.6,  78.2)
y9 <- c(88.8,  89.1,  87.7,  80.9)
y10 <-c(89.8,  90.0,  89.1,  82.9)

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

title(main="Minimal Job System efficiency wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="(%) Efficiency", col.lab=rgb(0,0,0))

legend(1, 45, c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);