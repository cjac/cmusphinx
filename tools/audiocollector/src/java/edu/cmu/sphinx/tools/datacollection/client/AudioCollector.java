package edu.cmu.sphinx.tools.datacollection.client;


import edu.cmu.sphinx.frontend.util.VUMeter;
import edu.cmu.sphinx.tools.audio.Player;
import edu.cmu.sphinx.tools.audio.Recorder;
import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.RegionOfAudioData;
import edu.cmu.sphinx.tools.corpus.Utterance;
import edu.cmu.sphinx.util.props.ConfigurationManager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.FieldPosition;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.ListIterator;

/**
 * Created by IntelliJ IDEA.
 * User: bertrand
 * Date: Apr 04, 2006
 * Time: 11:55:23 AM
 * The AudioController class contains all the logic and data for a datacollection session. Think of it as the
 * brains behind the scene. It has no presentation logic at all, and works like a transparent state machine
 * allowing modality specific renderers to leverage the same class.
 * Once created the modality specific application should drive this state machine with methods like "record" "next"
 * "play" "submit"etc... but once the session is finished "destroy" should always be called.
 */

public class AudioCollector {
    /*All the internal states of the AudioController*/
    final static protected byte BEFORE_RECORDING = 0;
    final static protected byte DURING_RECORDING = 1;
    final static protected byte AFTER_RECORDING = 2;
    final static protected byte DURING_PLAY = 3;
    final static protected byte INVALID_STATE = 4;

    protected byte _currentState = BEFORE_RECORDING;
    protected boolean _hasAudioData = false;
    protected Utterance _utterance;
    protected Corpus _corpus;
    protected ListIterator _uttIterator;
    protected Recorder _recorder;
    protected Player _player;


    protected static SimpleDateFormat filenameGenerator = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss");

    /**
     * This method returns a standardized name for each utterance based on the current time and subject
     * specific information
     *
     * @param userID from the Corpus
     * @param id     from the Corpus
     * @return a standardized filename for the utterance
     */
    protected static String getAudioFilename(String userID, String id) {
        StringBuffer sb = new StringBuffer(userID + "-" + id + "-");
        filenameGenerator.format(new Date(), sb, new FieldPosition(0));
        return sb.toString();
    }

    /**
     * This only constructor takes in a serializedCorpus (created by CorpusTranscriptOnlyWriter), and a
     * ConfigurationManager (with all the client specific information about low level recording)
     *
     * @param serializedCorpus a serialized corpus created by CorpusTranscriptsOnlyWriter
     * @param cm               ConfigurationManager
     */
    public AudioCollector(String serializedCorpus,
                           ConfigurationManager cm) {

        CorpusTranscriptsOnlyReader csr = new CorpusTranscriptsOnlyReader(serializedCorpus);
        _recorder = new Recorder();
        _recorder.open(cm);
        _corpus = csr.read();
        _uttIterator = _corpus.getUtterances().listIterator();
        _utterance = (Utterance) _uttIterator.next();
        _currentState = BEFORE_RECORDING;
    }

    /**
     * This is a cleanup method, that should invariably be called either at the end of the session or after the
     * session has been aborted for whatever reason.
     */
    public void destroy() {
        if (_recorder != null) _recorder.close();
    }


    private Corpus getFakeCorpus() {
        Corpus c = new Corpus();
        Utterance utt1 = new Utterance();
        utt1.setTranscript("test utterance one");
        Utterance utt2 = new Utterance();
        utt2.setTranscript("test utterance two");
        Utterance utt3 = new Utterance();
        utt3.setTranscript("test utterance three");
        Utterance utt4 = new Utterance();
        utt4.setTranscript("test utterance four");
        Utterance utt5 = new Utterance();
        utt5.setTranscript("test utterance five");

        c.addUtterance(utt1);
        c.addUtterance(utt2);
        c.addUtterance(utt3);
        c.addUtterance(utt4);
        c.addUtterance(utt5);

        return c;
    }

    /**
     * returns the internal state of the AudioController, may be used by the renderer's UI to inform
     * the user of their current action.
     *
     * @return BEFORE_RECORDING, DURING_RECORDING,AFTER_RECORDING,DURING_PLAY,INVALID_STATE
     */
    public byte getCurrentState() {
        return _currentState;
    }

    /**
     * returns the current VUMeter, if the AudioController is currently playing back an utterance
     * then that is the VUMeter that will get returned, otherwise it's the recorder's VUMeter
     *
     * @return current VUMeter based on the AudioController's state
     */
    public VUMeter getCurrentVUMeter() {
        if (_player != null && _player.isPlaying()) return _player.getVUMeter();
        else return _recorder.getVUMeter();
    }

    /**
     * If there currently is audio data captured for the current utterance returns true otherwise false.
     *
     * @return is there currently any audio data captured for the current utterance
     */
    public boolean hasCurrentAudioData() {
        return !(_utterance.getRegionOfAudioData() == null);
    }

    /**
     * The current transcript the subject is to read and utter
     *
     * @return current transcript
     */
    public String getCurrentTranscript() {
        if (_utterance == null) return "";
        else return _utterance.getTranscript();
    }

    /**
     * Based on the AudioController's current state, returns true only if "previous" is a valid request for that state.
     *
     * @return is "previous" currently a valid request?
     */
    public boolean isPreviousActive() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                return _uttIterator.hasPrevious();
            case DURING_RECORDING:
                return false;
            case AFTER_RECORDING:
                return _uttIterator.hasPrevious();
            case DURING_PLAY:
                return false;
            default:
                return false;
        }
    }

    /**
     * Based on the AudioController's current state, returns true only if "next" is a valid request for that state.
     *
     * @return is "next" currently a valid request?
     */
    public boolean isNextActive() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                return (_uttIterator.hasNext() && hasCurrentAudioData());
            case DURING_RECORDING:
                return false;
            case AFTER_RECORDING:
                return (_uttIterator.hasNext() && hasCurrentAudioData());
            case DURING_PLAY:
                return false;
            default:
                return false;
        }
    }

    /**
     * Based on the AudioController's current state, returns true only if "submit" is a valid request for that state.
     *
     * @return is "submit" currently a valid request?
     */
    public boolean isSubmitActive() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                return ((!_uttIterator.hasNext()) && hasCurrentAudioData());
            case DURING_RECORDING:
                return false;
            case AFTER_RECORDING:
                return ((!_uttIterator.hasNext()) && hasCurrentAudioData());
            case DURING_PLAY:
                return false;
            default:
                return false;
        }
    }

    /**
     * Based on the AudioController's current state, returns true only if "play" is a valid request for that state.
     *
     * @return is "play" currently a valid request?
     */
    public boolean isPlayActive() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                return hasCurrentAudioData();
            case DURING_RECORDING:
                return false;
            case AFTER_RECORDING:
                return hasCurrentAudioData();
            case DURING_PLAY:
                return false;
            default:
                return false;
        }
    }

    /**
     * Based on the AudioController's current state, returns true only if "record" is a valid request for that state.
     *
     * @return is "record" currently a valid request?
     */
    public boolean isRecordActive() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                return true;
            case DURING_RECORDING:
                return false;
            case AFTER_RECORDING:
                return true;
            case DURING_PLAY:
                return false;
            default:
                return false;
        }
    }

    /**
     * Based on the AudioController's current state, returns true only if "stop" is a valid request for that state.
     *
     * @return is "stop" currently a valid request?
     */
    public boolean isStopActive() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                return false;
            case DURING_RECORDING:
                return true;
            case AFTER_RECORDING:
                return false;
            case DURING_PLAY:
                return true;
            default:
                return false;
        }
    }

    /**
     * Should always be called only if isPlayActive() returns true. returns a Player object allowing the
     * renderer to playback the current utterance to the subject.  Calling stop() stops the playback.
     *
     * @return a player object for playback to the subject
     */
    public Player play() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                _currentState = INVALID_STATE;
                break;
            case DURING_RECORDING:
                _currentState = INVALID_STATE;
                break;
            case AFTER_RECORDING: {
                _currentState = DURING_PLAY;

                _player = new Player((_utterance.getRegionOfAudioData()));


                return _player;

            }
            case DURING_PLAY:
                _currentState = INVALID_STATE;
                break;
            default:
                _currentState = INVALID_STATE;
        }
        return null;
    }

    /**
     * Should always be called only if isRecordActive() returns true. returns a VUMeter object allowing the
     * renderer to display EQ for volume monitoring. calling stop() stops the recording.
     *
     * @return a VUMeter for volume monitoring
     */
    public VUMeter record() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                _currentState = DURING_RECORDING;
                break;
            case DURING_RECORDING:
                _currentState = INVALID_STATE;
                break;
            case AFTER_RECORDING:
                _currentState = DURING_RECORDING;
                break;
            case DURING_PLAY:
                _currentState = INVALID_STATE;
                break;
            default:
                _currentState = INVALID_STATE;
        }

        if (_currentState == DURING_RECORDING) {
            try {
                _recorder.start(getAudioFilename(_corpus.getProperty("userID"), _corpus.getProperty("ID")));
            } catch (IOException e) {
                e.printStackTrace();
            }
            return _recorder.getVUMeter();
        }
        return null;
    }

    /**
     * Should always be called only if isStopActive() returns true. stops playbacks started with play() or recordings
     * started with record().
     */
    public void stop() {
        switch (_currentState) {
            case BEFORE_RECORDING:
                _currentState = INVALID_STATE;
                break;
            case DURING_RECORDING: {
                _currentState = AFTER_RECORDING;
                try {
                    final RegionOfAudioData _regionOfAudioData = _recorder.stop();
                    _utterance.setRegionOfAudioData(_regionOfAudioData);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                break;
            }

            case AFTER_RECORDING:
                _currentState = INVALID_STATE;
                break;
            case DURING_PLAY:
                _currentState = AFTER_RECORDING;
                break;

            default:
                _currentState = INVALID_STATE;
        }

    }

    /**
     * Should always be called only if isPreviousActive() returns true. Gets the previous utterance from the corpus
     * and makes that the current utterance for playback or rerecording or rereading the transcript.
     */
    public void previous() {
        _utterance = (Utterance) _uttIterator.previous();
        _currentState = AFTER_RECORDING;
    }

    /**
     * Should always be called only if isNextActive() returns true. Gets the next utterance from the corpus
     * and makes that the current utterance for recording.
     */
    public void next() {

        _utterance = (Utterance) _uttIterator.next();
        _currentState = BEFORE_RECORDING;
    }

    /**
     * Should always be called only if isSubmitActive() returns true. Gathers all the utterances recorded in
     * the session stuffs them in a zip file, does the same for the a serialized version of the corpus in XML
     * format, and submits it to the URL it's being passed
     * @param url to which the data will be posted to
     */
    public void submit(URL url) {
        try {
            HttpURLConnection urlCon = (HttpURLConnection) url.openConnection();
            urlCon.setDoOutput(true);
            urlCon.setDoInput(true);
            // urlCon.setRequestProperty("Content-Encoding", "application/zip");
            urlCon.setRequestMethod("POST");
            urlCon.setRequestProperty("Content-type", "application/zip");
            urlCon.setAllowUserInteraction(false);
            urlCon.setUseCaches(false);


            OutputStream out = urlCon.getOutputStream();

            CorpusXMLZipWriter cxzw = new CorpusXMLZipWriter(out);

            cxzw.write(_corpus);
            out.flush();
            out.close();
            urlCon.connect();
            BufferedReader in = new BufferedReader(new InputStreamReader(urlCon.getInputStream()));

            String retVal = in.readLine();
            
            in.close();
            System.out.println("=======> Wrote zip file to the outputstream returned: " + retVal);
            _currentState = INVALID_STATE;
        }
        catch (Exception e) {
            e.printStackTrace();
        }

    }

}