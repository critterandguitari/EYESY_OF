print("Hello World!")

knob1 = 120 
knob2 = 120 
knob3 = 120 
knob4 = 120 
knob5 = 120

image = of.Image()

----------------------------------------------------
function setup()
	of.setWindowTitle("knobs example")
	print("script setup")
	
	of.enableNormalizedTexCoords()
	
	image:load("/home/music/openFrameworks/apps/myApps/eyesy/bin/data/images/tdf_1972_poster.jpg")
	
    of.setLineWidth(4);
end

----------------------------------------------------
function update()
end

----------------------------------------------------
function draw()

	of.pushMatrix()

	local boxSize = knob5 + 10

	of.translate(500,500)
	of.rotateXDeg(knob2)
	of.rotateYDeg(knob3)
	of.rotateZDeg(knob4)

	of.fill()

    image:bind();
	of.setColor(255,255,255)
--	of.drawBox(boxSize)
	of.drawSphere(boxSize)
	image:unbind();
	
	
	of.noFill()
	of.setColor(255,100,255)
	of.drawBox(boxSize * 2)
	

	of.popMatrix()
end

----------------------------------------------------
function exit()
	print("script finished")
end



