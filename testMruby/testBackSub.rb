puts "Start Test"
$lb = LibVision.new()
=begin
$lb.set_value4key(["imagePath", "/home/pi/Develop/LibVision/testMruby/background.png"])
$lb.execute(["loadImageFromMem"])
$lb.execute(["setCurrentBackground"])
$lb.set_value4key(["imagePath", "/home/pi/Develop/LibVision/testMruby/testIn.png"])
$lb.execute(["loadImageFromMem"])
$lb.execute(["subtractBackground"])
$lb.imagePath "/home/pi/Develop/LibVision/testMruby/background.png"
$lb.execute(["loadImageFromMem"])
=end

# Missing method test
$lb.loadImageFromMem "/home/pi/Develop/LibVision/testMruby/background.png"
$lb.setCurrentBackground
$lb.loadImageFromMem "/home/pi/Develop/LibVision/testMruby/testIn.png"
$lb.subtractBackground
$lb.saveFrame "/home/pi/test.png"