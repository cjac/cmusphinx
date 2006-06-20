<%--
  Created by IntelliJ IDEA.
  User: bertrand
  Date: Mar 23, 2006
  Time: 1:04:44 PM
  uses the information entered by the Subject to authenticate them, if they pass the authentication
  we create a corpus and stuff the session with subject information and that corpus and pass it to
  the audio_collector.jsp
--%>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<%@ page import="edu.cmu.sphinx.tools.datacollection.server.Subject" %>
<%@ page import="edu.cmu.sphinx.tools.corpus.Corpus" %>

<%
    StringBuffer reqURL = request.getRequestURL();
    reqURL.delete(reqURL.lastIndexOf("/"), reqURL.length());
    reqURL.delete(reqURL.lastIndexOf("/"), reqURL.length());
    Subject.init(reqURL.toString() + "/configs/");
%>
<html>
<head>
    <title>Text Entry Project - Data Collection Login</title>
</head>

<body>
Logging <%= request.getParameter("userID") == null ? "" : request.getParameter("userID")%>...
<%
    final String userID = request.getParameter("userID");
    final String password = request.getParameter("password");
    if ((userID == null) || (password == null) || !(Subject.isValidLogin(userID, password))) {
%>
<jsp:forward page="login.jsp">
    <jsp:param name="msg" value="invalid USER or PASSWORD"/>
</jsp:forward>
<%
} else {
    Subject subject = new Subject(request.getParameter("userID"));
    if (!(subject.getStatus().equals("ok"))) {
%>
<jsp:include page="login.jsp" flush="true">
    <jsp:param name="msg" value="<%= subject.getStatus()%>"/>
</jsp:include>
<%
        } else {
            /*Let's stuff the session with: information about the subject to display
      * the corpus itself so we can write audio to it
      * the stateful list of transcripts */
            Corpus myCorpus = subject.getCorpus();
            myCorpus.setProperty("userID", subject.getUserID());
            myCorpus.setProperty("ID", Integer.toString(subject.getID()));
            session.setAttribute("subject", subject);
            session.setAttribute("corpus", myCorpus);
            response.sendRedirect("audio_collector.jsp");

        }
    }
%>

</body>
</html>