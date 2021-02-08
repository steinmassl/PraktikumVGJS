x <- c(2,4,8,16)
y1 <- c(1.57927,   3.30885,   4.38725,   1.93718)
y2 <- c(1.78601,   3.64264,   6.92692,   6.07645)
y3 <- c(1.83946,   3.75863,   7.25364,   13.4699)
y4 <- c(1.88665,   3.82631,   7.43305,   14.5302)
y5 <- c(1.90871,   3.85719,   7.55804,   14.8459)
y6 <- c(1.92124,   3.88193,   7.63005,   15.0404)
y7 <- c(1.93331,   3.90219,   7.67823,   15.1843)
y8 <- c(1.94386,   3.91841,   7.71903,   15.272 )
y9 <- c(1.95121,   3.92209,   7.75461,   15.3641)
y10 <-c(1.95587,   3.93222,   7.78444,   15.4314)

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

title(main="(Locked) Function pointer Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("1us","2us","3us","4us","5us", "6us", "7us", "8us","9us"), 
   col=c("black","brown","purple", "blue", "red", "green", "orange", "yellow", "pink"), 
	pch=21, lty=1, cex=1, lwd=2);