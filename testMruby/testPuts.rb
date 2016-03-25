puts "START LIB"
$lb = LibVision.new()
$lb.set_value4key(["imagePath", "/home/pi/testIn.png"])
$lb.set_value4key(["savedImagePath", "default.png"])

puts "END LIB"

puts "START PUTS"

for ip in 0..1500
  #$lb.set_value4key(["imagePath", "/home/pi/testIn.png"])
  #$lb.set_value4key(["savedImagePath", "default.png"])
  $lb.execute(["loadImageFromMem"])
  $lb.execute(["preprocessingADPT"])
  $lb.execute(["detectSquares"])
  $lb.execute(["drawCandidates"])
  $lb.execute(["saveFrame"])
  $lb.execute(["preprocessingADPT"])
  $lb.execute(["detectCircles"])
  $lb.execute(["drawCandidates"])
  $lb.testDebug
  puts ip
end
puts "END PUTS"
