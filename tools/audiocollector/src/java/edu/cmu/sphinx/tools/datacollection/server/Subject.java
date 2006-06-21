package edu.cmu.sphinx.tools.datacollection.server;


import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.Utterance;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Locale;
import java.util.Properties;


/**
 * Created by IntelliJ IDEA.
 * User: bertrand
 * Date: Mar 24, 2006
 * Time: 11:55:23 AM
 * Identifies and authenticates and represents a Subject, the person whose voice is recorded during the session
 *
 */

public class Subject {
    private static boolean INITIALIZED = false;
    private static Properties SUBJECTS_PROPERTIES;
    private static String _resourceDirectory;
    protected String _userID = null;
    protected Locale _locale = null;
    protected int _passwordHashCode = 0;
    protected String _characterEncoding = null;
    protected String _status = null;
    protected int _myHashCode = 0;
    protected Corpus _corpus = null;


    /**
     * Should be called at init time.
     * Creates a thread that reads the content of the Subjects based on a file "subjects.xml"
     * whose location is specified by @param directory. The contents of which are reread
     * every @param refreshRate millis
     * @param directory
     * @param refreshRate
     */
   public static synchronized void init(final String directory,
                                        final int refreshRate) {
       if (!INITIALIZED) {
           _resourceDirectory = directory;
           Thread t = new Thread() {
               public void run() {
                   try {
                       InputStream fis = (new URL(_resourceDirectory + "subjects.xml")).openStream();
                       SUBJECTS_PROPERTIES = new Properties();
                       SUBJECTS_PROPERTIES.loadFromXML(fis);
                       fis.close();
                       INITIALIZED = true;
                   } catch (java.util.InvalidPropertiesFormatException e) {
                       e.printStackTrace();
                   } catch (java.io.IOException e) {
                       e.printStackTrace();
                   }
                   try {
                       sleep(refreshRate);
                   } catch (java.lang.InterruptedException e) {
                       e.printStackTrace();
                   }
               }
           };
           t.setDaemon(true);
           t.start();
       }
   }

    /**
     * Same as init except that the default refresh rate is each minute
     * @param directory
     */
    public static void init(final String directory) {
                 init(directory,1000*60*1);
    }


    public int hashCode() {
        return _myHashCode;
    }

    /**
     * Given a userID and a password validates the user based on the information read by the init method
     * @param userID  userID as it appears in the subjects.xml
     * @param password  password as it appears in the subjects.xml
     * @return  true if the userID exists and matches the given password, false otherwise
     */
    public static boolean isValidLogin(String userID,
                                       String password) {
        while (!INITIALIZED) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

     return ((SUBJECTS_PROPERTIES.getProperty("subjects").indexOf(userID)>=0)&&
             (SUBJECTS_PROPERTIES.getProperty(userID+".password")!=null)&&
             password.equals(SUBJECTS_PROPERTIES.getProperty(userID+".password")));
    }

    /**
     * the user has been validate through isValidLong than a Subject object may be created with just the userID
     * @param userID
     */
    public Subject(String userID) {
        final String myUserID = userID + ".";
        _userID = userID;
        _passwordHashCode = (SUBJECTS_PROPERTIES.getProperty(myUserID+"password")).hashCode();
        _locale = new Locale(SUBJECTS_PROPERTIES.getProperty(myUserID+"locale"));
        _characterEncoding = SUBJECTS_PROPERTIES.getProperty(myUserID+"encoding");
        _status = SUBJECTS_PROPERTIES.getProperty(myUserID+"status");
        _myHashCode = Integer.parseInt(SUBJECTS_PROPERTIES.getProperty(myUserID+"ID"));

    }

    /**
     * gets the UserID as it appears in the given subjects.xml file
     * @return the userID
     */
    public String getUserID() {
        return _userID;
    }

    /**
     * gets the subject's locale as it appears in the given subjects.xml file
     * @return the locale object
     */
    public Locale getLocale() {
        return _locale;
    }

     /**
     * gets the subject's Character Set preference as it appears in the given subjects.xml file
     * @return the charset String
     */
    public String getCharset() {
        return _characterEncoding;
    }

    /**
     * gets the subject's account's status as it appears in the given subjects.xml file
     * @return Account status (0 means there are no issues)
     */
    public String getStatus() {
        return _status;
    }

     /**
     * sets the subject's account's status (but does not write it to the subjects.xml)
      * used for session scoped stati/
     */
    public void setStatus(String status) {
        _status = status;
    }

     /**
     * gets the subject's unique ID as it appears in the given subjects.xml file
     * @return unique ID
     */
    public int getID() {
        return _myHashCode;
    }

    /**
     *  gets value of a generic property as it appears in the given subjects.xml file
     * @param name of the property
     * @return  value of the property
     */
    public String getProperty(String name) {
        return (SUBJECTS_PROPERTIES.getProperty(getUserID() + "." + name));
    }

    /**
     * Creates a transcripts only corpus to commence the data collection session
     * this is the method to override for a more sophisticated selection of utterance based
     * on the subjects profile. the default implementation returns the same set of transcripts
     * for all Subjects sharing the same locale.
     * @return a corpus object containing only transcripts
     */
    public Corpus getCorpus() {
        if (_corpus==null) {
            _corpus = new Corpus();
            try {
                InputStream fis = (new URL(_resourceDirectory+"transcripts_"+getLocale()+".txt")).openStream();
                InputStreamReader isr = new InputStreamReader(fis,getCharset());
                BufferedReader in = new BufferedReader(isr);

                String line;
                while ((line=in.readLine()) != null) {
                    Utterance utt = new Utterance();
                    utt.setTranscript(line);
                    _corpus.addUtterance(utt);
                }

                fis.close();
                isr.close();
                in.close();
            } catch (java.io.IOException e) {
                e.printStackTrace();
            }

        }
        return _corpus;
    }

}

