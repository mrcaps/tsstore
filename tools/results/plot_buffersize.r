library(plyr)
library(ggplot2)

setwd(dirname(sys.frame(1)$ofile))

bsize = read.csv("buffersize.csv")

points = 354215*2
bsize$insertion_rate = points / bsize$itime 

number_ticks <- function(n) {function(limits) pretty(limits, n)}
ggplot(bsize, aes(x=bufsize, y=insertion_rate/1e6, color=machine, linetype=machine)) + 
  geom_point() + geom_line() +
  scale_x_continuous("Buffer Size (points)") +
  scale_y_continuous("Insertion Rate\n (million points/sec)", breaks=number_ticks(5))

ggsave("plot_buffersize.pdf", width=5.2, height=2.4)


lambda = 100

dta = expand.grid(
  s = seq(1,32768,10),
  c = seq(1,5,1)
)

dta$d = 1 / (dta$c + lambda / dta$s)

ggplot(dta, aes(x=s, y=d, color=c, group=c)) + geom_line(width=3)