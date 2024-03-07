SELECT uri,
       title,
       pubdate
FROM Articles
WHERE id NOT IN
    (SELECT id
     FROM Articles
     ORDER BY -pubdate
     LIMIT ?)
ORDER BY -pubdate
limit ?;
