print("Hello World!")

counter = 0
bSmooth = false
max = 4
trans = 80
size = 100


----------------------------------------------------
function setup()
	of.setWindowTitle("graphics example")
	print("script setup")

	of.setCircleResolution(50)
	of.background(255, 255, 255, 255)
	of.setWindowTitle("graphics example")
	
	of.setFrameRate(60) -- if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps
	of.disableSmoothing()
	--of.disableAlphaBlending()
end

----------------------------------------------------
function update()
	counter = counter + 1
end

----------------------------------------------------
function draw()

	for i=1,max do
	    x = size *(max-i)
	    y = size *(max-i)
	
        
        of.setColor(of.random(0,255), of.random(0, 255), of.random(0, 255), trans)
    	of.pushMatrix()
	    	of.translate(640, 540, 0)
	    	of.rotateDeg(counter, 0, 0, 1)
	    	of.drawRectangle(-x/2, -y/2, size*(max-i), size*(max-i))
	    of.popMatrix()
	
	    
	    of.setColor(of.random(0,255), of.random(0, 255), of.random(0, 255),trans)
	    of.pushMatrix()
		    of.translate(640, 540, 0)
		    of.rotateDeg(counter*-1, 0, 0, 1)
		    of.drawRectangle(-x/2, -y/2, size*(max-i), size*(max-i))
	    of.popMatrix()
	    
	    of.setColor(of.random(0,255), of.random(0, 255), of.random(0, 255), trans)
    	of.pushMatrix()
	    	of.translate(1280, 540, 0)
	    	of.rotateDeg(counter, 0, 0, 1)
	    	of.drawRectangle(-x/2, -y/2, size*(max-i), size*(max-i))
	    of.popMatrix()
	
	    
	    of.setColor(of.random(0,255), of.random(0, 255), of.random(0, 255),trans)
	    of.pushMatrix()
		    of.translate(1280, 540, 0)
		    of.rotateDeg(counter*-1, 0, 0, 1)
		    of.drawRectangle(-x/2, -y/2, size*(max-i), size*(max-i))
	    of.popMatrix()
	    
	end
	
	of.pushMatrix()
	    of.translate(960, 540, 0)
	    of.setHexColor(0xffffff)
		of.drawRectangle(0, 0, 3, 3)
	of.popMatrix()
	
	
	
	--for i=0,2 do
	--	of.setColor(of.random(0, 255), of.random(0, 255), of.random(0, 255))
	--	of.drawRectangle(of.random(25, 350), of.random(35, 450),
	--	                 of.random(100, 200), of.random(100, 200))
	--end
	of.setHexColor(0x000000)
	of.drawBitmapString("oh my cool", 275, 500)
	-- label
	of.setHexColor(0x000000)
	of.drawBitmapString("nice", 75, 500)


end

----------------------------------------------------
function exit()
	print("script finished")
end



