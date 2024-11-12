window.cServer = (function(){
  const URL_BASE = window.location.origin;
  const URL_USER_SOURCES = URL_BASE + "/api/user-sources/1";
  const URL_SOURCE_ARTICLES = URL_BASE + "/api/source-articles-paged/[SOURCE_ID]/[PAGE_NR]";
  const URL_SEARCH_ARTICLES = URL_BASE + "/api/search-user-articles-paged/1/[SEARCH_TEXT]/[PAGE_NR]";

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

  function getPageInfo(path) {
    const splitPath = path.split("/")
    let sourceId, page, search;
    if (splitPath.length === 4) {
      if (splitPath[1] === "search") {
        search = splitPath[2];
      } else {
        sourceId = splitPath[2];
      }
      page = parseInt(splitPath[3], 10);
    } else {
      if (splitPath[1] === "search") {
        search = splitPath.pop();
      } else {
        sourceId = splitPath.pop();
      }
      page = 0;
    }
    return [sourceId, search, page]
  }

  async function navToLocationPath() {
    const [sourceId, search, page] = getPageInfo(location.pathname)
    let state;
    if (search) {
      state = {
        articles: await fetchSearchArticles(search, page),
        page: page,
        search: search,
      }
    } else {
      state = {
        articles: await fetchArticles(sourceId, page),
        page: page,
        sourceId: sourceId,
      }
    }
    history.replaceState(state, "", location.pathname);
    displayContent(state);
  }

  async function navToSource(evt) {
    evt.preventDefault();
    const [sourceId, search, page] = getPageInfo(evt.target.pathname)
    let state;
    if (search) {
      state = {
        title: evt.target.textContent,
        articles: await fetchSearchArticles(search, page),
        page: page,
        search: search
      }
    }
    else {
      state = {
        title: evt.target.textContent,
        articles: await fetchArticles(sourceId, page),
        page: page,
        sourceId: sourceId,
      }
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
    const url = URL_SOURCE_ARTICLES.replace("[SOURCE_ID]", source_id).replace("[PAGE_NR]", page_nr);
    const resp = await fetch(url);
    const data = await resp.json();

    const articles = data.results;
    return articles;
  }

  async function fetchSearchArticles(text, page_nr) {
    const url = URL_SEARCH_ARTICLES.replace("[SEARCH_TEXT]", text).replace("[PAGE_NR]", page_nr);
    const resp = await fetch(url);
    const data = await resp.json();

    const articles = data.results;
    return articles;
  }

  async function handleFormSearch(evt) {
    evt.preventDefault()
    const text = evt.target.querySelector('input[type="text"]').value;
    const state = {
      title: evt.target.textContent,
      articles: await fetchSearchArticles(text, 0),
      page: 0,
      search: text,
    }
    history.pushState(state, "", "/search/" + text + "/0");
    displayContent(state);
  }

  function displayContent(state) {
    const TPL_ARTICLE = document.getElementById("tplArticle");
    const TARGET_ARTICLES = document.querySelector("#feed-items");

    const T_PREV = document.getElementById("prevPage");
    const T_NEXT = document.getElementById("nextPage");

    if (state.title) {
      document.title = state.title;
    }

    const articles = state.articles;

    if (state.page >= 1) {
      if (state.search) {
        T_PREV.href = `/search/${state.search}/${state.page - 1}`
      } else {
        T_PREV.href = `/source/${state.sourceId}/${state.page - 1}`
      }
      T_PREV.style.visibility = 'visible'
    } else {
      T_PREV.style.visibility = 'hidden'
    }

    if (articles.length) {
      if (state.search) {
        T_NEXT.href = `/search/${state.search}/${state.page + 1}`
      } else {
        T_NEXT.href = `/source/${state.sourceId}/${state.page + 1}`
      }
      T_NEXT.style.visibility = 'visible'
    } else {
      T_NEXT.style.visibility = 'hidden'
    }

    if (articles.length) {

      TARGET_ARTICLES.innerHTML = "";

      for (let i = 0; i < articles.length; i++) {
        const elItem = TPL_ARTICLE.content.cloneNode(true);
        const elTitle = elItem.querySelector(".feed-item-title");
        const elDate = elItem.querySelector(".feed-item-date");
        const elSource = elItem.querySelector(".feed-item-source");

        elTitle.textContent = articles[i].title;
        elTitle.href = articles[i].uri;

        elDate.textContent = formatDateValue(articles[i].date);
        elSource.textContent = new URL(articles[i].uri).hostname;

        TARGET_ARTICLES.appendChild(elItem);
      }

    }
  }

  function init() {
    const T_PREV = document.getElementById("prevPage");
    const T_NEXT = document.getElementById("nextPage");
    const FORM_SEARCH = document.getElementById("search");

    FORM_SEARCH.addEventListener("submit", handleFormSearch);

    T_PREV.addEventListener("click", navToSource);
    T_NEXT.addEventListener("click", navToSource);

    navToLocationPath()
  }

  return {
    init,
    fetchSources,
  }

})()
