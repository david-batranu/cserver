window.cServer = (function(){
  const URL_USER_SOURCES = "http://cserver:8080/user-sources/1";
  const URL_SOURCE_ARTICLES = "http://cserver:8080/source-articles-paged/[SOURCE_ID]/[PAGE_NR]";

  function formatDateValue(value) {
    const intl = new Intl.DateTimeFormat("ro-RO", { month: "short" });
    const date = new Date(parseInt(value, 10) * 1000);
    const month = intl.format(date);
    const day = date.getDate();
    const year = date.getFullYear();
    const time = date.toLocaleTimeString("ro-RO", { timeStyle: "short" });
    return `${month} ${day}, ${year} ${time}`
  }

  function handlePopState(evt) {
    if (evt.state) {
      displayContent(evt.state);
    }
  }

  window.addEventListener("popstate", handlePopState);

  async function navToSource(evt) {
    evt.preventDefault();
    const sourceId = evt.target.pathname.split("/").pop()
    const state = {
      title: evt.target.textContent,
      articles: await fetchArticles(sourceId, 0),
    }
    history.pushState(state, "", evt.target.pathname);
    displayContent(state);
  }

  async function fetchSources() {
    const TPL_SOURCE = document.getElementById("tplSource");
    const TARGET_SOURCES = document.querySelector("#main aside nav ul");

    const resp = await fetch(URL_USER_SOURCES);
    const data = await resp.json();

    const sources = data.results;

    for (let i = 0; i < sources.length; i++) {
      const elItem = TPL_SOURCE.content.cloneNode(true);
      const elA = elItem.querySelector("a");
      elA.textContent = sources[i].title;
      elA.href = "/source/" + sources[i].id;
      elA.addEventListener("click", navToSource);
      TARGET_SOURCES.appendChild(elItem);
    }
  }

  async function fetchArticles(source_id, page_nr) {
    const url = URL_SOURCE_ARTICLES.replace("[SOURCE_ID]", source_id).replace("[PAGE_NR]", page_nr)
    const resp = await fetch(url);
    const data = await resp.json();

    const articles = data.results;
    console.log(articles);
    return articles;
  }

  function displayContent(state) {
    const TPL_ARTICLE = document.getElementById("tplArticle");
    const TARGET_ARTICLES = document.querySelector("#feed-items");

    document.title = state.title;

    const articles = state.articles;

    console.log(TARGET_ARTICLES);
    TARGET_ARTICLES.innerHTML = "";

    for (let i = 0; i < articles.length; i++) {
      const elItem = TPL_ARTICLE.content.cloneNode(true);
      const elTitle = elItem.querySelector(".feed-item-title");
      const elDate = elItem.querySelector(".feed-item-date");

      elTitle.textContent = articles[i].title;
      elTitle.href = articles[i].uri;

      elDate.textContent = formatDateValue(articles[i].date);

      TARGET_ARTICLES.appendChild(elItem);
    }
  }

  return {
    fetchSources,
  }

})()
