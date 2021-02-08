x <- c(2,4,8,16)
y1 <- c(1.40219,   2.96076,   1.41199,   0.809795)
y2 <- c(1.67238,   3.43614,   6.41042,   1.70315)
y3 <- c(1.77905,   3.62399,   6.91919,   2.78528) 
y4 <- c(1.83677,   3.72272,   7.19556,   4.55693)
y5 <- c(1.87447,   3.7835 ,   7.36786,   13.2461)
y6 <- c(1.9002 ,   3.82921,   7.49149,   14.5523)
y7 <- c(1.91767,   3.85979,   7.5726 ,   14.8464)
y8 <- c(1.93123,   3.88074,   7.63696,   15.002 )
y9 <- c(1.94228,   3.90022,   7.68841,   15.1563)
y10 <-c(1.94926,   3.88369,   7.71902,   15.2715)

g_range <- range(1, 16)

plot(x, y1, lwd=2, type="o", col="black",
ylim=g_range, xlim=g_range, axes=FALSE, ann=FALSE)

lines(x, y2, lwd=2, type="o", col="brown")
lines(x, y3, lwd=2, type="o", col="purple")
lines(x, y4, lwd=2, type="o", col="blue")
lines(x, y5, lwd=2, type="o", col="red")
lines(x, y6, lwd=2, type="o", col="green")
lines(x, y7, lwd=2, type="o", col="orange")
lines(x, y8, lwd=2, type="o", col="yellow")
lines(x, y9, lwd=2, type="o", col="pink")

axis(1, las=1, at=1:16)
axis(2, las=1, at=1:16)

grid(nx=NA, ny=NULL, col="gray")

box()

title(main="(Lock-free) Coroutine Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);