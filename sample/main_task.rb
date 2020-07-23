def log(msg)
  @logger.write(msg)
end
  
@logger = F401re::Log.new()
@sleep = F401re::Task.new()
log("$$ main task initialize\r\n")

F401re::Task.active(F401re::TASK1_ID)
log("$$ main task task1 active\r\n")
log("$$ main task 1\r\n")

i = 0
while true
  i = i + 1
  log("$$ main task : #{i}\r\n")
  i2 = 0
  while true
    break if i2 == 300000
    i2 += 1
  end
  F401re::Task.wakeup(F401re::TASK1_ID)
end
