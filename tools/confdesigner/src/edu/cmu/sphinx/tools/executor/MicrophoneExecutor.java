package edu.cmu.sphinx.tools.executor;

import edu.cmu.sphinx.util.props.*;
import edu.cmu.sphinx.recognizer.Recognizer;
import edu.cmu.sphinx.frontend.util.Microphone;
import edu.cmu.sphinx.result.Result;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class MicrophoneExecutor implements Executable {

    @S4Component(type = Recognizer.class)
    public static final String PROP_RECOGNIZER = "recognizer";

    @S4Integer(defaultValue = 10, range = {1, Integer.MAX_VALUE})
    public static final String PROP_RESULT_TIMOUT = "numTimeOutResults";

    private Recognizer recognizer;
    private int numTimeOutResults;
    private String name;


    public void doExecute() {
        Microphone microphone = getMicroPhone();

        /* allocate the resource necessary for the recognizer */
        try {
            recognizer.allocate();
        } catch (Exception e) {
            System.out.println ("Failed to allocate recognizer");
            e.printStackTrace();
        }

        // the microphone will keep recording until the program exits
        System.out.println("Please say something !");
        if (microphone.startRecording()) {


            while (true) {
                System.out.println("Start speaking. Press Ctrl-C to quit.\n");

                /*
                 * This method will return when the end of speech
                 * is reached. Note that the endpointer will determine
                 * the end of speech.
                 */
                Result result = recognizer.recognize();

                if (result != null) {
                    String resultText = result.getBestFinalResultNoFiller();
                    System.out.println("You said: " + resultText + "\n");
                } else {
                    System.out.println("I can't hear what you said.\n");
                }
            }
        }

    }


    public void newProperties(PropertySheet ps) throws PropertyException {
        recognizer = (Recognizer) ps.getComponent(PROP_RECOGNIZER);
        numTimeOutResults = ps.getInt(PROP_RESULT_TIMOUT);
        name = ps.getInstanceName();
    }


    public String getName() {
        return null;
    }

     public static Microphone getMicroPhone() {
        Microphone mic = new Microphone();

        PropertySheet propSheet = new PropertySheet(mic, null, new RawPropertyData("tt", mic.getClass().getName()), new ConfigurationManager());
        try {
            propSheet.setString(Microphone.PROP_STEREO_TO_MONO, "selectChannel");
            propSheet.setInt(Microphone.PROP_SELECT_CHANNEL, 2);
            propSheet.setBoolean(Microphone.PROP_BIG_ENDIAN, true);
//            propSheet.setLogger(getLogger());

            mic.newProperties(propSheet);
            mic.initialize();

            return mic;
        } catch (PropertyException e) {
            e.printStackTrace();
        }

        return null;
    }
}
