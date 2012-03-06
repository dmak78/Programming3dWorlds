var newVertices : Vector3[] = new Vector3[273];
var newNormals : Vector3[] = new Vector3[273];
var newUV : Vector2[] = new Vector2[273];;
public var newTriangles : int[] = new int[819];

function Start () {
    var mesh : Mesh = new Mesh ();
    GetComponent(MeshFilter).mesh = mesh;
    mesh.vertices = newVertices;
    mesh.normals = newNormals;
    mesh.uv = newUV;
    mesh.triangles = newTriangles;
}

function Update () {
}

function OSCMessageReceived(message : OSC.NET.OSCMessage){
	//Debug.Log("I got a message! " + message.Values[0]);
	
	var mesh = gameObject.GetComponent(MeshFilter).mesh;
	
	if(message.Address == "/vectors"){
	
	
	
   		var numVertices : int = 0;

		for(var j = 0 ; j < message.Values.Count-9; j+=3){
			newVertices[numVertices].x = message.Values[j];
			newVertices[numVertices].y = message.Values[j+1];
			newVertices[numVertices].z = message.Values[j+2];
			newUV[numVertices].x = message.Values[j];
			newUV[numVertices].y = message.Values[j+2];
			newTriangles[j] = numVertices;
			newTriangles[j+1] = numVertices+1;
			newTriangles[j+2] = numVertices+2;
			numVertices++;
			
		}
//		for(var t = 0; t < newTriangles.Length;t++){
//			newTriangles[t] = t;
//		}
		

		Debug.Log(newVertices[0].x + " " +  newVertices[0].y + " " + newVertices[0].z);
		
		mesh.vertices=newVertices;
		mesh.uv = newUV;
		mesh.triangles = newTriangles;
		
	if(message.Address == "/normals"){
    	var numNormVertices : int = 0;
		for(var i = 0 ; i< message.Values.Count-3; i+=3){
			
			newVertices[numNormVertices].x = message.Values[i];
			newVertices[numNormVertices].y = message.Values[i+1];
			newVertices[numNormVertices].z = message.Values[i+2];
			//newUV[numVertices].x = message.Values[j];
			//newUV[numVertices].y =message.Values[j+2];
			
			numNormVertices++;
		}
			mesh.normals=newNormals;
			
		mesh.RecalculateBounds();
		mesh.RecalculateNormals();
		mesh.Optimize();
		
//			message.Values[0] + " " + message.Values[1] + " " + message.Values[2]);
//			//do something interesting
//			var targetPosition : Vector3 = Vector3( Map(message.Values[0], min, max, -10.0, 10.0, true),
//								  					Map(message.Values[1], min, max, -10.0, 10.0, true),
//							  						Map(message.Values[2], min, max, -10.0, 10.0, true));  
////			 transform.position = 
//			//transform.position += (targetPosition - transform.position) * .01;
//			transform.position = Vector3.Lerp(transform.position, targetPosition,  smoothing);
			
		}
	//	else{
	//		Debug.LogError("/read/accelerometer has the wrong number of args");
	//	}
	}
}					



function Map(value : float, inputMin : float, inputMax : float, outputMin : float, outputMax : float , clamp : boolean) : float 
{
	if (Mathf.Abs(inputMin - inputMax) < Mathf.Epsilon){
		//Debug.Log("Map: avoiding possible divide by zero, check inputMin and inputMax");
		return outputMin;
	} else {
		var outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);	
		if( clamp ){
			if(outputMax < outputMin){
				if( outVal < outputMax )outVal = outputMax;
				else if( outVal > outputMin )outVal = outputMin;
			}else{
				if( outVal > outputMax )outVal = outputMax;
				else if( outVal < outputMin )outVal = outputMin;
			}
		}
		return outVal;
	}
}
