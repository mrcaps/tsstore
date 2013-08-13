library(plyr)
library(ggplot2)

setwd(dirname(sys.frame(1)$ofile))

bsize = read.csv("buffersize.csv")

points = 354215*2
bsize$insertion_rate = points / bsize$itime 

number_ticks <- function(n) {function(limits) pretty(limits, n)}
ggplot(bsize, aes(x=bufsize, y=insertion_rate/1e6)) + 
  geom_point() + geom_line() +
  scale_x_continuous("Buffer Size (points)") +
  scale_y_continuous("Insertion Rate\n (million points/sec)", breaks=number_ticks(5))

ggsave("plot_buffersize.pdf", width=3.2, height=2.4)