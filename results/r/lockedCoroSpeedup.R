x <- c(2,4,8,16)
y1 <- c(1.61288,   3.29757,   3.99292,   3.3056 )
y2 <- c(1.82719,   3.65205,   7.02952,   9.10407)
y3 <- c(1.87846,   3.76754,   7.35472,   13.0504) 
y4 <- c(1.91094,   3.82129,   7.527  ,   14.5256)
y5 <- c(1.92864,   3.86019,   7.63125,   14.9627)
y6 <- c(1.94247,   3.89115,   7.70433,   15.19  )
y7 <- c(1.95369,   3.90823,   7.75779,   15.3427)
y8 <- c(1.96304,   3.92357,   7.80082,   15.4581)
y9 <- c(1.96996,   3.94033,   7.82919,   15.5201)
y10 <-c(1.97412,   3.94244,   7.84457,   15.5999)

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

title(main="(Locked) Coroutine Speedup wrt sequential execution", font.main=2)

title(xlab="(#) Threads")
title(ylab="Speedup", col.lab=rgb(0,0,0))

legend(1, g_range[2], c("1us","2us","3us","4us","5us","6us","7us","8us","9us"), 
   col=c("black","brown","purple","blue","red","green","orange","yellow","pink"), 
	pch=21, lty=1, cex=1, lwd=2);