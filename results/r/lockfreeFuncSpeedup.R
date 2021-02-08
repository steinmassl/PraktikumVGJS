x <- c(2,4,8,16)
y1 <- c(1.35653,   2.87575,   1.11554,   0.73214)
y2 <- c(1.64896,   3.37991,   4.60299,   1.47851)
y3 <- c(1.72082,   3.57799,   6.70122,   2.28024)
y4 <- c(1.81677,   3.6831 ,   7.02006,   3.16948)
y5 <- c(1.84858,   3.74601,   7.20281,   4.15193)
y6 <- c(1.87363,   3.78749,   7.32988,   5.40325)
y7 <- c(1.8923 ,   3.82227,   7.43443,   9.54994)
y8 <- c(1.90534,   3.84806,   7.50611,   14.6715)
y9 <- c(1.91532,   3.8677 ,   7.56231,   14.8356)
y10 <-c(1.92507,   3.88369,   7.59813,   14.978 )

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
#lines(x, y10,lwd=2, type="o", col="cadetblue")

axis(1, las=1, at=1:16)
axis(2, las=1, at=1:16)

grid(nx=NA, ny=NULL, col="gray")

box()

title(main="(Lock-free) Function Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);