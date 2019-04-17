import numpy as np
import cv2 as cv2
import os as os

# An implementation for finding key points in a face.
# Uses an already existing implementation of SIFT

# Erik Bertse
# Training data is omitted here.

from sklearn.svm import LinearSVC

click_xy = [0,0]
clicked_kp_desc = []

# click callback
def click(event, x, y, flags, param):
	if (event == cv2.EVENT_LBUTTONDOWN):
		click_xy[0] = x;
		click_xy[1] = y;
		print ("Click at: ") 
		print("  -- x:  " + str(x) + ",  -- y:  " + str(y));

class FeatureReader:

	def __init__ (self):

		image_size = (640,480)
		self.background = np.zeros((image_size[1],image_size[0],3))

		# the openCV window
		cv2.namedWindow('Webcam', cv2.WINDOW_AUTOSIZE)
		cv2.namedWindow('face', cv2.WINDOW_AUTOSIZE)

		# the trained haar-cascade classifier data
		self.face_cascade = cv2.CascadeClassifier('frontal_face_features.xml')

		self.train_feats = []
		self.train_lbls = []

		# Training data for the SVMs
		files = ["olec", "oleb", "mleb", "ileb", "orec", "ireb", "mleb", "ileb", "lmc", "rmc", "ilec", "ilec", "ulom"]

		for filename in files:
			train_lec_feats_pos, train_lec_lbls_pos = self.getFeatsAndLablesFromFile("images/sift_data/" + filename + "_pos.txt",1)
			train_lec_feats_neg, train_lec_lbls_neg = self.getFeatsAndLablesFromFile("images/sift_data/" + filename + "_neg.txt",0)
	

			self.train_feats.append(np.concatenate((train_lec_feats_pos, train_lec_feats_neg)))
			self.train_lbls.append(np.concatenate((train_lec_lbls_pos, train_lec_lbls_neg)))

		# Create the SIFT detector
		self.sift = cv2.xfeatures2d.SIFT_create(edgeThreshold = 20)

		# Start streaming
		self.wc = cv2.VideoCapture(0)

		cv2.setMouseCallback("face", click)
		
		self.svm_rec = LinearSVC(tol = 0.000001)
		self.svms = []

		# Create the SVMSs
		for feats,lbls in zip(self.train_feats,self.train_lbls):
			svm = LinearSVC(tol = 0.000001)
			svm.fit(feats,lbls)
			self.svms.append(svm)
		

	# Helper function that returns features and lables.
	def getFeatsAndLablesFromFile(self, filename, posneg):
		thefile = open(filename, "a+")
		train_feats = []
		train_lbls = []

		lines = thefile.readlines()
		lines = [line.rstrip('\n') for line in lines]

		for line in lines: 
			numbers = line.split()
			numbers = [float(num) for num in numbers]
			train_feats.append(numbers)
			if (posneg == 1):
				train_lbls.append(int(1))
			else:
				train_lbls.append(int(0))

		train_feats = np.asarray(train_feats)
		train_lbls = np.asanyarray(train_lbls)
		thefile.close()

		return train_feats, train_lbls

	# Returns the coordinates of the best matching point out of 9
	#  around the initial guess (x,y)
	def getBestPixelOfNine(self, img, x,y, r, svm, stepbystep):
			picks = []
			kps = []

			keypoint_size = 5
			kps.append(cv2.KeyPoint(x, y, keypoint_size))
			picks.append([x,y])
			
			kps.append(cv2.KeyPoint(x, y+r, keypoint_size))
			picks.append([x,y+r])
			
			kps.append(cv2.KeyPoint(x, y-r, keypoint_size))
			picks.append([x,y-r])

			kps.append(cv2.KeyPoint(x-r, y, keypoint_size))
			picks.append([x-r,y])

			kps.append(cv2.KeyPoint(x-r, y+r, keypoint_size))
			picks.append([x-r,y+r])

			kps.append(cv2.KeyPoint(x-r, y-r, keypoint_size))
			picks.append([x-r,y-r])

			kps.append(cv2.KeyPoint(x+r, y, keypoint_size))
			picks.append([x+r,y])

			kps.append(cv2.KeyPoint(x+r, y+r, keypoint_size))
			picks.append([x+r,y+r])

			kps.append(cv2.KeyPoint(x+r, y-r, keypoint_size))
			picks.append([x+r,y-r])

			out_kp, desc = self.sift.compute(img, kps)
			img = cv2.drawKeypoints(img,kps,img)

			if stepbystep:
				for i in range(len(picks)):
					cv2.rectangle(img, (picks[i][0], picks[i][1]), (picks[i][0]+1,picks[i][1]+1), (0,0,255), 5)
					cv2.imshow('face', img)
				cv2.waitKey(0)

			confidence_scores = svm.decision_function(desc)
			pos = np.argmax(confidence_scores)
			cv2.rectangle(img, (picks[pos][0], picks[pos][1]), (picks[pos][0]+1, picks[pos][1]+1), (0,0,255), 2)
			return picks[pos][0], picks[pos][1]


	# Helper function that writes features to a file.
	def writeFeatsToFile(self, feats, file_obj):
		for row in feats:
			outstring = ""
						
			for num in row:
				outstring += str(num) + " "
			outstring += "\n"
			
			file_obj.write(outstring)



	def frame (self):
		ret, frame = self.wc.read()
		if len(frame) < 1:
			raise RuntimeError('Frame skipped')
		# image to display
		image = np.asanyarray(frame).astype(np.float32)
		image -= np.min(image[:])
		image /= np.max(image[:])
		#convert to grayscale
		gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
		gray_image -= np.min(gray_image[:])
		gray_image /= np.max(gray_image[:])
		gray_image_uint = gray_image * 255
		gray_image_uint = gray_image_uint.astype(np.uint8)

		# the face detection
		mask = np.ones((image.shape[0],image.shape[1]))
		faces = self.face_cascade.detectMultiScale(gray_image_uint, 1.3, 5)

		# Process faces
		if len(faces) > 0:
			
			(x,y,w,h) = faces[0]
			face_xdim = 300
			face_ydim = 300
			face_img = cv2.resize(gray_image_uint[y:y+h, x:x+w], (face_xdim,face_ydim))

			k = cv2.waitKey(500)

			# Best first initial guesses
			best_pixels = [ [80,120], [80,100], [100,95], [120,70],    [220,120], [180,100], [200,95], [220,100],     [100,260], [200,260], [120,120], [200,120], [150,230]]

			#Timer
 			e1 = cv2.getTickCount()


 			# For each feature, return a more promising point while step is positive.
			for i in range(len(self.train_feats)):	
				step = 25
				while (step > 0):
					a,b = self.getBestPixelOfNine(face_img, best_pixels[i][0], best_pixels[i][1], step, self.svms[i], False)
					best_pixels[i] = [a,b]
					if step < 4:
						step = step - 3
					elif step < 12:
						step = step - 7
					else:
						step = step - 10

			e2 = cv2.getTickCount()
			time = (e2 - e1)/ cv2.getTickFrequency()
			print("time: " + str(time))

			# Draw points on the face image
			for [xpix,ypix] in best_pixels:
				cv2.rectangle(face_img, (xpix, ypix), (xpix+1, ypix+1), (0,0,255), 2)

			cv2.imshow('face', face_img)

			# Code for collecting descriptors from clicked points
			if (k == 120):  #the x key 
				k = cv2.waitKey(0)
				if (k == 115): #the s key
					keyp = cv2.KeyPoint(click_xy[0], click_xy[1], 5)
					out_kp, desc = self.sift.compute(face_img, [keyp])
					clicked_kp_desc.append(desc[0])
					print("Appended keypoint: ")
					print(str(desc[0]))

			for (x,y,w,h) in faces:
				# draw a rectangle where a face is detected
				cv2.rectangle(image, (x,y),(x+w,y+h), (255,0,0), 2)

				redshade = 0
				blueshade = 0

				for [xpix,ypix] in best_pixels:
					# Draw result guesses
					cv2.rectangle(image, (x + xpix*w/face_xdim, y + ypix*h/face_ydim), (x + xpix*w/face_xdim+1,  y + ypix*h/face_ydim+1), (0,blueshade,redshade), 7)
					blueshade = blueshade + 0.05
					redshade += 0.05

			# Write the clicked features
			if (k == 119): #the w key

				filename = "images/sift_data/ulom_neg.txt"
				print("Writing feature data to:   " + filename)

				with open(filename, "a") as file_obj: 
					self.writeFeatsToFile(clicked_kp_desc, file_obj)

		# alpha blending
		image = mask[...,None] * image + (1 - mask[...,None]) * self.background

		# Show images
		cv2.imshow('Webcam', image)
		cv2.waitKey(1)

	def finalize (self):
	   	# Stop streaming
		self.wc.release()
		cv2.destroyAllWindows()


A = FeatureReader()
try:
	while True:
		A.frame() 
except KeyboardInterrupt:
	A.finalize()
	print("\nShutting down -- Good Bye")

