
                  Dim MyFile As New FileStream("myfile.txt", FileMode.Open, FileAccess.Read, FileShare.Read)Dim MyArray(50) As ByteMyFile.Read(MyArray, 0, 50)DoDangerousOperation(MyArray(20))
               
               