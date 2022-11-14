import spacy

nlp = spacy.load('en_core_web_sm')

# From: https://subscription.packtpub.com/book/data/9781838987312/2/ch02lvl1sec16/extracting-subjects-and-objects-of-the-sentence
# With slight modification
def get_object_phrase(doc):
    for token in doc:
        if ("dobj" in token.dep_):
            subtree = list(token.subtree)
            start = subtree[0].i
            end = subtree[-1].i + 1
            return doc[start:]

def extract(sentence):
    doc = nlp(sentence)
    phrase = get_object_phrase(doc)
    return phrase
