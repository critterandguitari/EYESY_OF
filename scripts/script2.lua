print("Hello World!")

knob1 = 120 
knob2 = 120 
knob3 = 120 
knob4 = 120 
knob5 = 120

----------------------------------------------------
function setup()
	of.setWindowTitle("knobs example")
	print("script setup")
    of.setLineWidth(4);
    of.enableDepthTest();
end

----------------------------------------------------
function update()
end

----------------------------------------------------
function draw()
    local movementSpeed = .1 * (knob1 / 1000)
	local cloudSize = 1200--of.getWidth() / 2
	local maxBoxSize = 100
	local spacing = 1
	local boxCount = 100
	
	for i=0,boxCount do

		of.pushMatrix()

		local t = (of.getElapsedTimef() + i * spacing) * movementSpeed
		local pos = glm.vec3(of.signedNoise(t, 0, 0), of.signedNoise(0, t, 0), of.signedNoise(0, 0, t))



		boxSize = maxBoxSize * of.noise(pos.x, pos.y, pos.z)

		pos = pos * cloudSize
		of.translate(pos)
		of.rotateXDeg(pos.x)
		of.rotateYDeg(pos.y)
		of.rotateZDeg(pos.z)

	--	ofLogo.bind();
		of.fill()
		of.setColor(255)
		of.drawBox(boxSize)
		--ofLogo.unbind();

		of.noFill()
		of.setColor(20,100,200)
		of.drawBox(boxSize * 1.1)

		of.popMatrix()
	end
end

----------------------------------------------------
function exit()
	print("script finished")
end


