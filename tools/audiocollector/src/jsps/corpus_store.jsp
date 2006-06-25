<%--
  Created by IntelliJ IDEA.
  User: bertrand
  Date: May 10, 2006
  Time: 1:04:44 PM
  This is a non displayable jsp responsible for receiving the zipped streamed of finished sessions and stores them
  on the server as files.
  it also returns the number of bytes returned for confirmation.
--%>
<%@ page import="java.io.FileOutputStream"%>
<%@ page import="java.io.InputStream"%>

<%
    try {
    final String dir = application.getInitParameter("collected_data_directory");
    final String filename = dir+"/"+request.getParameter("userID")+"-"+request.getParameter("ID")+System.currentTimeMillis()+".zip";
    FileOutputStream fos = new FileOutputStream(filename);
    byte[] chunk = new byte[1024];
    int totalLength = 0;
    int chunkLength = 0;
    InputStream in = request.getInputStream();
    while ((chunkLength = in.read(chunk))> 0) {
        fos.write(chunk, 0, chunkLength);
        totalLength += chunkLength;
    }
    out.println(totalLength);
    System.out.println("Wrote "+totalLength+" bytes in "+filename);
    fos.close();
    } catch (Exception e) {
        e.printStackTrace();
    }
%>