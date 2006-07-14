<%--
  Created by IntelliJ IDEA.
  User: bertrand
  Date: Mar 23, 2006
  Time: 1:04:44 PM
  This is the main jsp, after authentication serves the user with the applet for data collection and passes
  the appropritate corpus and server side info (location of configuration manager, store jsp for resulting utts etc)
  This jsp also sets the html page whose DOM will be manipulated by the applet.
--%>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<%@ page import="edu.cmu.sphinx.tools.datacollection.server.Subject" %>
<%@ page import="edu.cmu.sphinx.tools.datacollection.server.CorpusTranscriptsOnlyWriter"%>
<%@ page import="edu.cmu.sphinx.tools.corpus.Corpus"%>

<%!
    static String isDisabled(boolean active) {
        if (!active) return "DISABLED";
        else return "";
    }

    static String getCorpusAsString(Corpus corpus) {
        StringBuffer sbuf = new StringBuffer();
        CorpusTranscriptsOnlyWriter csw = new CorpusTranscriptsOnlyWriter(sbuf);
        csw.write(corpus);
        return sbuf.toString();
    }

    static String getServletURL(HttpServletRequest request ,String filename) {
        StringBuffer reqURL = request.getRequestURL();
        reqURL.delete(reqURL.lastIndexOf("/"),reqURL.length());
        reqURL.delete(reqURL.lastIndexOf("/"),reqURL.length());
        return reqURL.toString()+"/"+filename;
    }

%>
<%    if (session.isNew())  {   %>
    <jsp:forward page="login.jsp">
         <jsp:param name="msg" value="Session Timed Out! Please Login again" />
     </jsp:forward>
<% } %>

<%

    final Subject subject = (Subject)session.getAttribute("subject");
    final Corpus corpus = (Corpus)session.getAttribute("corpus");
%>


<html>
<center>
    <head>
        <title>Text Entry Project - Data Collection</title>
    </head>

    <body>
    <h2><i>AudioCollector (Text Entry Project) - <%=subject.getUserID()%></i></h2>

    <form action="audio_collector.jsp" name="billboard">

        <textarea name="transcript"
                  rows="2"
                  cols="50"
                  style="color:blue; text-align:center; font-size:25; font-weight:bold; background-color:#E8E8E8"
                  READONLY>
            Loading...
        </textarea>
    </form>

    <hr>

    <form action="audio_collector.jsp" name="eq" width="80%">
       <table cellspacing="0" cellpadding="0" >
            <tr >
                <td width="133" NOWRAP>
                       <input type=text
                           name="average-hi"
                           size="25"
                           maxlength="25"
                           value=""
                           style="color:red;font-size:7; font-weight:bold; background-color:black; text-align:left"
                           READONLY/>



                    <input type=text
                           name="average-mid"
                           size="25"
                           maxlength="25"
                           value=""
                           style="color:orange;font-size:7; font-weight:bold; background-color:black; text-align:left"
                           READONLY/>

                     <input type=text
                           name="average-normal"
                           size="25"
                           maxlength="25"
                           value=""
                           style="color:yellow;font-size:7; font-weight:bold; background-color:black; text-align:left"
                           READONLY/>


                </td>
            </tr>
        </table>
    </form>

    <jsp:plugin type="applet"

                name="audiocontroller"
                jreversion="1.5"
                codebase=".."
                archive="datacollection-client.jar,sphinx4.jar,corpus.jar,javolution.jar,batch.jar"
                code="edu.cmu.sphinx.tools.datacollection.client.AudioCollectorPresentationApplet"
                width="400"
                height="36">
        <jsp:params>
            <jsp:param name="mayscript" value="true"/>
            <jsp:param name="corpus" value="<%=getCorpusAsString(corpus)%>"/>
            <jsp:param name="config" value="<%=getServletURL(request, "configs/recorder.config.xml")%>"/>
            <jsp:param name="store" value="<%=getServletURL(request, "jsps/corpus_store.jsp")%>"/>
        </jsp:params>
    </jsp:plugin>

    <hr>

    <font SIZE=3><a href="login.jsp?logout">logout</a></font>
    </body>

</center>
</html>