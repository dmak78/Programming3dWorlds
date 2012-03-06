var newVertices : Vector3[];
var oldVertices : Vector3[];
var numVertices : int = 0;
var orgVertexCount : int;
var newUV : Vector2[];
var newTriangles : int[];
var triVertexCount : int = 0;
var triangleCount : int = 0;

		
function Start () {
	mesh = gameObject.GetComponent(MeshFilter).mesh;

		newVertices = mesh.vertices;
		oldVertices = mesh.vertices;
		
   		newUV = mesh.uv;
   		
    	newTriangles = mesh.triangles;

		orgVertexCount = mesh.vertexCount;

}

function Update () {

}

function OSCMessageReceived(message : OSC.NET.OSCMessage){
	//Debug.Log("I got a message! " + message.Values[0]);

	
	if(message.Address == "/vectors"){
	
	mesh = gameObject.GetComponent(MeshFilter).mesh;
		
		numVertices = 0;
		triVertexCount=0;
		triangleCount=0;
		for(var j = 0 ; j < message.Values.Count; j+=3){
			newVertices[numVertices].x = Mathf.Lerp(oldVertices[numVertices].x, message.Values[j], .2 );
			newVertices[numVertices].y = Mathf.Lerp(oldVertices[numVertices].y, message.Values[j+1], .2 );
			newVertices[numVertices].z = Mathf.Lerp(oldVertices[numVertices].z, message.Values[j+2], .2 );
			newUV[numVertices].x = message.Values[j];
			newUV[numVertices].y = message.Values[j+1];
			numVertices++;
			//newTriangles = [numVertices,numVertices+1,numVertices+2];
		}
		for(var x = numVertices ; x < orgVertexCount; x++){
			newVertices[x].x = newVertices[100].x;
			newVertices[x].y = newVertices[100].y;
			newVertices[x].z = newVertices[100].z;
			newUV[x].x = newUV[100].x;
			newUV[x].y = newUV[100].y;
		}

		Debug.Log(newVertices[100].x + " " +  newVertices[100].y + " " + newVertices[100].z);
		mesh.vertices=newVertices;
		oldVertices=newVertices;
		mesh.uv = newUV;
	
		
	if(message.Address == "/normals"){
	
		mesh = gameObject.GetComponent(MeshFilter).mesh;
    	var numNormVertices : int = 0;
		var newNormals : Vector3[];
		newNormals = mesh.normals;

		for(var i = 0 ; i< message.Values.Count-3; i+=3){
			
			newNormals[numNormVertices].x = message.Values[i];
			newNormals[numNormVertices].y = message.Values[i+1];
			newNormals[numNormVertices].z = message.Values[i+2];
			
			numNormVertices++;
		}
		mesh.normals=newNormals;
	
//		mesh.RecalculateBounds();
//		mesh.RecalculateNormals();
//		mesh.Optimize();
			
		}


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
