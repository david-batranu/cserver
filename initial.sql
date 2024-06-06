CREATE TABLE user_greeting(name TEXT, greeting TEXT);
CREATE TABLE Users(id INTEGER PRIMARY KEY, name TEXT);
CREATE TABLE Sources(id INTEGER PRIMARY KEY, uri TEXT, title TEXT);
CREATE TABLE UserSources(
    id INTEGER PRIMARY KEY,
    userid INTEGER,
    sourceid INTEGER,
    FOREIGN KEY(sourceid) REFERENCES Sources(id)
    FOREIGN KEY(userid) REFERENCES Users(id)
);
CREATE TABLE Articles(
    id INTEGER PRIMARY KEY,
    uri TEXT,
    title TEXT,
    description TEXT,
    pubdate INTEGER,
    date INTEGER,
    sourceid INTEGER,
    FOREIGN KEY(sourceid) REFERENCES Sources(id)
);

INSERT INTO Sources VALUES(0, "https://some.site/feed", "Some site");

INSERT INTO Users VALUES(NULL, "David");

INSERT INTO UserSources VALUES(NULL, 1, 1);
INSERT INTO UserSources VALUES(NULL, 1, 2);

INSERT INTO user_greeting VALUES("david", "Hello, David!");
