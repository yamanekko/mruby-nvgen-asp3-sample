def log(msg)
  @logger.write(msg)
end
  
@logger = F401re::Log.new()
log("$$-- task1 active\r\n")
while true
  log("$$-- task1 wakeup : \r\n")
  i2 = 0
  while true
    break if i2 == 1000
    i2 += 1
  end
  F401re::Task.sleep
end
F401re::Task.exit
