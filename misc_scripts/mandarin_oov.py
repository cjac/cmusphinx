#!/usr/bin/env python

import sys
import codecs

def read_arpa(arpa):
    fh = codecs.open(arpa, "r", "utf8")
    indata = False
    in1gram = False
    words = set()
    for spam in fh:
        if spam == u"\\data\\\n":
            indata = True
            continue
        if indata and spam == u"\\1-grams:\n":
            in1gram = True
            continue
        if in1gram and spam == u"\\2-grams:\n":
            break
        if spam == u"\n":
            continue
        if in1gram:
            prob, word, bo = spam.split()
            words.add(word)
    return words

def find_best_oov(vocab, words):
    """
    Find a minimum OOV segmentation of words, returning the number of
    OOVs and word count of that segmentation.
    """
    # Expand words into a graph based on vocab.  Each section of
    # unparsed characters becomes an arc with cost N where N is the
    # length of the section.  In-vocabulary words have cost 0.  Note
    # that this means we will not get a unique segmentation of the
    # in-vocabulary portions.
    chars = u"".join(words)

    # Create one state for each position in the word plus a start state
    states = []
    for i in range(0, len(chars)):
        states.append([])

    # For each position in the sentence, create arcs for every word
    # starting at that position, and sort them longest-first
    for i in range(0, len(chars)):
        for j in range(i+1, len(chars)+1):
            # Create an arc for this word
            if chars[i:j] in vocab:
                states[i].append((0, chars[i:j]))
            else:
                # FIXME: Could probably make these implicit?
                states[i].append((j-i, chars[i:j]))
        states[i].sort(reverse=True)

    # Find the shortest path through that graph
    state_costs = [999999] * (len(states) + 1)
    state_prevs = [0] * (len(states) + 1)
    state_costs[0] = 0
    Q = range(0, len(states))
    while Q:
        # Find lowest-cost state
        pcost = 999999
        for i, stateid in enumerate(Q):
            if state_costs[stateid] < pcost:
                pcost = state_costs[stateid]
                break
        del Q[i]
        # Expand its outgoing arcs, updating costs, prevs
        for cost, word in states[stateid]:
            destid = stateid + len(word)
            dcost = pcost + cost
            if dcost < state_costs[destid]:
                state_costs[destid] = dcost
                state_prevs[destid] = stateid
    # Backtrace to get segmentation and OOVs (since the segmentation
    # counts an OOV segment as a single word, we do too)
    pos = len(state_prevs)-1
    seg = []
    oov = []
    while pos > 0:
        bp = state_prevs[pos]
        word = chars[bp:pos]
        cost = state_costs[pos] - state_costs[bp]
        seg.insert(0, word)
        if cost > 0:
            oov.append(word)
        pos = bp
    return seg, oov

if __name__ == '__main__':
    try:
        arpa, txt = sys.argv[1:]
    except:
        sys.stderr.write("Usage: %s ARPA TXT\n" % sys.argv[0])
        sys.exit(2)

    vocab = read_arpa(arpa)
    oovset = {}
    noov = 0
    nwords = 0
    for spam in codecs.open(txt, "r", "utf8"):
        words = spam.split()
        # Remove start and end sentence markers (and possibly uttid)
        if words[0] == "<s>":
            del words[0]
        for i in range(len(words)-1, -1, -1):
            if words[i] == "</s>":
                del words[i:]
                break
        if words[-1].startswith('('):
            del words[-1:]
        seg, oov = find_best_oov(vocab, words)
        for w in oov:
            oovset[w] = oovset.get(w, 0) + 1
        noov += len(oov)
        nwords += len(seg)
        print u" ".join(seg).encode('utf8')
    print "%d words, %d oovs, OOV rate %.2f%%" % (nwords, noov, float(noov)/nwords*100)
    if noov > 0:
        print "OOVs and counts:"
        for w, c in sorted(oovset.items(), key=lambda x: x[1], reverse=True):
            print w.encode('utf8'), c
